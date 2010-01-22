// $Id$
/*****************************************************************************
 *   Copyright (C) 2007-2009 by Bernd Flemisch                               *
 *   Copyright (C) 2008-2010 by Markus Wolff                                 *
 *   Institute of Hydraulic Engineering                                      *
 *   University of Stuttgart, Germany                                        *
 *   email: <givenname>.<name>@iws.uni-stuttgart.de                          *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version, as long as this copyright notice    *
 *   is included in its original form.                                       *
 *                                                                           *
 *   This program is distributed WITHOUT ANY WARRANTY.                       *
 *****************************************************************************/
#ifndef DUNE_CAPILLARYDIFFUSION_HH
#define DUNE_CAPILLARYDIFFUSION_HH

#include "dumux/transport/fv/diffusivepart.hh"

/**
 * @file
 * @brief  Class for defining the diffusive capillary pressure term of a saturation equation
 * @author Bernd Flemisch, Markus Wolff
 */
namespace Dune
{
/*!\ingroup diffPart
 * @brief  Class for defining the diffusive capillary pressure term of a saturation equation
 *
 * Defines the diffusive capillary pressure term of the form
 * \f[
 *  \bar \lambda \boldsymbol{K} \text{grad} \, p_c,
 * \f]
 * where \f$\bar \lambda = \lambda_w f_n = \lambda_n f_w\f$ and \f$\lambda\f$ is a phase mobility and \f$f\f$ a phase fractional flow function,
 * \f$\boldsymbol{K}\f$ is the intrinsic permeability and \f$p_c = p_c(S_w) \f$ the capillary pressure.
 *
 * Template parameters are:

 - GridView      a DUNE gridview type
 - Scalar        type used for scalar quantities
 - VC            type of a class containing different variables of the model
 - Problem       class defining the physical problem
 */
template<class TypeTag>
class CapillaryDiffusion: public DiffusivePart<TypeTag>
{
private:
    typedef typename GET_PROP_TYPE(TypeTag, PTAG(GridView)) GridView;
      typedef typename GET_PROP_TYPE(TypeTag, PTAG(Scalar)) Scalar;
      typedef typename GET_PROP_TYPE(TypeTag, PTAG(Problem)) Problem;
      typedef typename GET_PROP(TypeTag, PTAG(ReferenceElements)) ReferenceElements;
      typedef typename ReferenceElements::Container ReferenceElementContainer;

      typedef typename GET_PROP_TYPE(TypeTag, PTAG(TwoPIndices)) Indices;

      typedef typename GET_PROP_TYPE(TypeTag, PTAG(SpatialParameters)) SpatialParameters;
      typedef typename SpatialParameters::MaterialLaw MaterialLaw;

      typedef typename GET_PROP_TYPE(TypeTag, PTAG(FluidSystem)) FluidSystem;
      typedef typename GET_PROP_TYPE(TypeTag, PTAG(PhaseState)) PhaseState;

    enum
    {
        dim = GridView::dimension, dimWorld = GridView::dimensionworld
    };
    enum
    {
        wPhaseIdx = Indices::wPhaseIdx, nPhaseIdx = Indices::nPhaseIdx
    };

    typedef typename GridView::Traits::template Codim<0>::Entity Element;
    typedef typename GridView::template Codim<0>::EntityPointer ElementPointer;
    typedef typename GridView::IntersectionIterator IntersectionIterator;
    typedef Dune::FieldVector<Scalar, dim> FieldVector;
    typedef Dune::FieldVector<Scalar, dim> LocalPosition;
    typedef Dune::FieldVector<Scalar, dimWorld> GlobalPosition;
    typedef Dune::FieldMatrix<Scalar,dim,dim> FieldMatrix;

public:
    //! Returns capillary diffusion term
    /*! Returns capillary diffusion term for current element face
     *  @param[in] element        entity of codim 0
     *  @param[in] indexInInside  face index in reference element
     *  @param[in] satI           saturation of current element
     *  @param[in] satJ           saturation of neighbor element
     *  @param[in] pcGradient     gradient of capillary pressure between element I and J
     *  \return     capillary pressure term of the saturation equation
     */
    virtual FieldVector operator() (const Element& element, const int indexInInside, Scalar satI, Scalar satJ, const FieldVector& pcGradient) const
    {
        // cell geometry type
        GeometryType gt = element.geometry().type();

        // cell center in reference element
        const LocalPosition& localPos = ReferenceElementContainer::general(gt).position(0,0);

        // get global coordinate of cell center
        const GlobalPosition& globalPos = element.geometry().global(localPos);

        // get absolute permeability of cell
        FieldMatrix permeability(problem_.spatialParameters().intrinsicPermeability(globalPos,element));

        IntersectionIterator isItEnd = problem_.gridView().template iend(element);
        IntersectionIterator isIt = problem_.gridView().template ibegin(element);
        for (; isIt != isItEnd; ++isIt)
        {
            if(isIt->indexInInside() == indexInInside)
            break;
        }
        int globalIdxI = problem_.variables().index(element);

        // get geometry type of face
        GeometryType faceGT = isIt->geometryInInside().type();

        //get lambda_bar = lambda_n*f_w
        Scalar mobBar = 0;
        Scalar mobilityWI = 0;
        Scalar mobilityNWI = 0;

        if (preComput_)
        {
            mobilityWI = problem_.variables().mobilityWetting(globalIdxI);
            mobilityNWI = problem_.variables().mobilityNonwetting(globalIdxI);
        }
        else
        {
            PhaseState phaseState;
            phaseState.update(problem_.temperature(globalPos, element));
            mobilityWI = MaterialLaw::krw(problem_.spatialParameters().materialLawParams(globalPos, element), satI);
            mobilityWI /= FluidSystem::phaseViscosity(wPhaseIdx, phaseState);
            mobilityNWI = MaterialLaw::krn(problem_.spatialParameters().materialLawParams(globalPos, element), satI);
            mobilityNWI /= FluidSystem::phaseViscosity(nPhaseIdx, phaseState);
        }

        if (isIt->neighbor())
        {
            // access neighbor
            ElementPointer neighborPointer = isIt->outside();

            int globalIdxJ = problem_.variables().index(*neighborPointer);

            // compute factor in neighbor
            GeometryType neighborGT = neighborPointer->geometry().type();
            const LocalPosition& localPosNeighbor = ReferenceElementContainer::general(neighborGT).position(0,0);

            // neighbor cell center in global coordinates
            const GlobalPosition& globalPosNeighbor = neighborPointer->geometry().global(localPosNeighbor);

            // distance vector between barycenters
            FieldVector distVec = globalPosNeighbor - globalPos;

            // compute distance between cell centers
            Scalar dist = distVec.two_norm();

            FieldVector unitDistVec(distVec);
            unitDistVec /= dist;

            // get absolute permeability
            FieldMatrix permeabilityJ(problem_.spatialParameters().intrinsicPermeability(globalPosNeighbor, *neighborPointer));

            // harmonic mean of permeability
            for (int x = 0;x<dim;x++)
            {
                for (int y = 0; y < dim;y++)
                {
                    if (permeability[x][y] && permeabilityJ[x][y])
                    {
                        permeability[x][y]= 2*permeability[x][y]*permeabilityJ[x][y]/(permeability[x][y]+permeabilityJ[x][y]);
                    }
                }
            }
            Scalar mobilityWJ = 0;
            Scalar mobilityNWJ = 0;
            //get lambda_bar = lambda_n*f_w
            if(preComput_)
            {
                mobilityWJ = problem_.variables().mobilityWetting(globalIdxJ);
                mobilityNWJ = problem_.variables().mobilityNonwetting(globalIdxJ);
            }
            else
            {
                PhaseState phaseState;
                phaseState.update(problem_.temperature(globalPosNeighbor, *neighborPointer));
                mobilityWJ = MaterialLaw::krw(problem_.spatialParameters().materialLawParams(globalPosNeighbor, *neighborPointer), satJ);
                mobilityWJ /= FluidSystem::phaseViscosity(wPhaseIdx, phaseState);
                mobilityNWJ = MaterialLaw::krn(problem_.spatialParameters().materialLawParams(globalPosNeighbor, *neighborPointer), satJ);
                mobilityNWJ /= FluidSystem::phaseViscosity(nPhaseIdx, phaseState);
            }
            Scalar mobilityWMean = 0.5*(mobilityWI + mobilityWJ);
            Scalar mobilityNWMean = 0.5*(mobilityNWI + mobilityNWJ);
            mobBar = mobilityWMean*mobilityNWMean/(mobilityWMean+mobilityNWMean);
         }//end intersection with neighbor
        else
        {
            Scalar mobilityWJ = 0;
            Scalar mobilityNWJ = 0;

            //calculate lambda_n*f_w at the boundary
            PhaseState phaseState;
            phaseState.update(problem_.temperature(globalPos, element));
            mobilityWJ = MaterialLaw::krw(problem_.spatialParameters().materialLawParams(globalPos, element), satJ);
            mobilityWJ /= FluidSystem::phaseViscosity(wPhaseIdx, phaseState);
            mobilityNWJ = MaterialLaw::krn(problem_.spatialParameters().materialLawParams(globalPos, element), satJ);
            mobilityNWJ /= FluidSystem::phaseViscosity(nPhaseIdx, phaseState);

            Scalar mobWMean = 0.5 * (mobilityWI + mobilityWJ);
            Scalar mobNWMean = 0.5 * (mobilityNWI + mobilityNWJ);

            mobBar = mobWMean * mobNWMean / (mobWMean + mobNWMean);
        }

        // set result to K*grad(pc)
        FieldVector result(0);
        permeability.umv(pcGradient, result);

        // set result to f_w*lambda_n*K*grad(pc)
        result *= mobBar;

        return result;
    }

    /*! @brief Constructs a CapillaryDiffusion object
     *  @param problem an object of class Dune::TransportProblem or derived
     *  @param soil implementation of the solid matrix
     *  @param preComput if preCompute = true previous calculated mobilities are taken, if preCompute = false new mobilities will be computed (for implicit Scheme)
     */
    CapillaryDiffusion (Problem& problem, const bool preComput = true)
    : DiffusivePart<TypeTag>(problem), problem_(problem), preComput_(preComput)
    {}

private:
    Problem& problem_;//problem data
    const bool preComput_;//if preCompute = true the mobilities are taken from the variable object, if preCompute = false new mobilities will be taken (for implicit Scheme)
};
}

#endif
