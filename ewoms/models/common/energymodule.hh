/*
  Copyright (C) 2008-2013 by Andreas Lauser
  Copyright (C) 2012 by Klaus Mosthaf

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
/*!
 * \file
 *
 * \brief Contains the classes required to consider energy as a
 *        conservation quantity in a multi-phase module.
 */
#ifndef EWOMS_ENERGY_MODULE_HH
#define EWOMS_ENERGY_MODULE_HH

#include <ewoms/disc/common/fvbaseproperties.hh>
#include <ewoms/models/common/quantitycallbacks.hh>

#include <dune/common/fvector.hh>

#include <string>

namespace Ewoms {
namespace Properties {
NEW_PROP_TAG(Indices);
NEW_PROP_TAG(EnableEnergy);
NEW_PROP_TAG(HeatConductionLaw);
NEW_PROP_TAG(HeatConductionLawParams);
}
}

namespace Ewoms {
/*!
 * \ingroup Energy
 * \brief Provides the auxiliary methods required for consideration of
 *        the energy equation.
 */
template <class TypeTag, bool enableEnergy>
class EnergyModule;

/*!
 * \copydoc Ewoms::EnergyModule
 */
template <class TypeTag>
class EnergyModule<TypeTag, /*enableEnergy=*/false>
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, Evaluation) Evaluation;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef typename GET_PROP_TYPE(TypeTag, RateVector) RateVector;
    typedef typename GET_PROP_TYPE(TypeTag, PrimaryVariables) PrimaryVariables;
    typedef typename GET_PROP_TYPE(TypeTag, ExtensiveQuantities) ExtensiveQuantities;
    typedef typename GET_PROP_TYPE(TypeTag, IntensiveQuantities) IntensiveQuantities;
    typedef typename GET_PROP_TYPE(TypeTag, Model) Model;
    typedef typename FluidSystem::ParameterCache ParameterCache;

    enum { numEq = GET_PROP_VALUE(TypeTag, NumEq) };

    typedef Dune::FieldVector<Evaluation, numEq> EvalEqVector;

public:
    /*!
     * \brief Register all run-time parameters for the energy module.
     */
    static void registerParameters()
    {}

    /*!
     * \brief Returns the name of a primary variable or an empty
     *        string if the specified primary variable index does not belong to
     *        the energy module.
     */
    static std::string primaryVarName(int pvIdx)
    { return ""; }

    /*!
     * \brief Returns the name of an equation or an empty
     *        string if the specified equation index does not belong to
     *        the energy module.
     */
    static std::string eqName(int eqIdx)
    { return ""; }

    /*!
     * \brief Returns the relative weight of a primary variable for
     *        calculating relative errors.
     */
    static Scalar primaryVarWeight(const Model &model,
                                   int globalDofIdx,
                                   int pvIdx)
    { return -1; }

    /*!
     * \brief Returns the relative weight of a equation of the residual.
     */
    static Scalar eqWeight(const Model &model,
                           int globalDofIdx,
                           int eqIdx)
    { return -1; }

    /*!
     * \brief Given a fluid state, set the temperature in the primary variables
     */
    template <class FluidState>
    static void setPriVarTemperatures(PrimaryVariables &priVars,
                                      const FluidState &fs)
    {}

    /*!
     * \brief Given a fluid state, set the enthalpy rate which emerges
     *        from a volumetric rate.
     */
    template <class FluidState>
    static void setEnthalpyRate(RateVector &rateVec,
                                const FluidState &fluidState,
                                int phaseIdx,
                                const Evaluation& volume)
    {}

    /*!
     * \brief Add the rate of the enthalpy flux to a rate vector.
     */
    static void setEnthalpyRate(RateVector &rateVec,
                                const Evaluation& rate)
    {}

    /*!
     * \brief Add the rate of the enthalpy flux to a rate vector.
     */
    static void addToEnthalpyRate(RateVector &rateVec,
                                  const Evaluation& rate)
    {}

    /*!
     * \brief Add the rate of the conductive heat flux to a rate vector.
     */
    static Evaluation heatConductionRate(const ExtensiveQuantities &extQuants)
    {
        typedef Opm::MathToolbox<Evaluation> Toolbox;

        return Toolbox::createConstant(0.0);
    }

    /*!
     * \brief Add the energy storage term for a fluid phase to an equation
     * vector
     */
    template <class LhsEval>
    static void addPhaseStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                const IntensiveQuantities &intQuants,
                                int phaseIdx)
    {}

    /*!
     * \brief Add the energy storage term for a fluid phase to an equation
     * vector
     */
    template <class LhsEval, class Scv>
    static void addFracturePhaseStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                        const IntensiveQuantities& intQuants,
                                        const Scv& scv,
                                        int phaseIdx)
    {}

    /*!
     * \brief Add the energy storage term for the fracture part a fluid phase to an
     *        equation vector
     */
    template <class LhsEval>
    static void addSolidHeatStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                    const IntensiveQuantities& intQuants)
    {}

    /*!
     * \brief Evaluates the advective energy fluxes over a face of a
     *        subcontrol volume and adds the result in the flux vector.
     *
     * This method is called by compute flux (base class)
     */
    template <class Context>
    static void addAdvectiveFlux(RateVector &flux,
                                 const Context &context,
                                 int spaceIdx,
                                 int timeIdx)
    {}

    /*!
     * \brief Evaluates the advective energy fluxes over a fracture
     *        which should be attributed to a face of a subcontrol
     *        volume and adds the result in the flux vector.
     */
    template <class Context>
    static void handleFractureFlux(RateVector &flux,
                                   const Context &context,
                                   int spaceIdx,
                                   int timeIdx)
    {}

    /*!
     * \brief Adds the diffusive heat flux to the flux vector over
     *        the face of a sub-control volume.
     *
     * This method is called by compute flux (base class)
     */
    template <class Context>
    static void addDiffusiveFlux(RateVector &flux,
                                 const Context &context,
                                 int spaceIdx,
                                 int timeIdx)
    {}
};

/*!
 * \copydoc Ewoms::EnergyModule
 */
template <class TypeTag>
class EnergyModule<TypeTag, /*enableEnergy=*/true>
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, Evaluation) Evaluation;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef typename GET_PROP_TYPE(TypeTag, EqVector) EqVector;
    typedef typename GET_PROP_TYPE(TypeTag, RateVector) RateVector;
    typedef typename GET_PROP_TYPE(TypeTag, PrimaryVariables) PrimaryVariables;
    typedef typename GET_PROP_TYPE(TypeTag, IntensiveQuantities) IntensiveQuantities;
    typedef typename GET_PROP_TYPE(TypeTag, ExtensiveQuantities) ExtensiveQuantities;
    typedef typename GET_PROP_TYPE(TypeTag, Indices) Indices;
    typedef typename GET_PROP_TYPE(TypeTag, Model) Model;
    typedef typename FluidSystem::ParameterCache ParameterCache;

    enum { numEq = GET_PROP_VALUE(TypeTag, NumEq) };
    enum { numPhases = FluidSystem::numPhases };
    enum { energyEqIdx = Indices::energyEqIdx };
    enum { temperatureIdx = Indices::temperatureIdx };

    typedef Dune::FieldVector<Evaluation, numEq> EvalEqVector;
    typedef Opm::MathToolbox<Evaluation> Toolbox;

public:
    /*!
     * \brief Register all run-time parameters for the energy module.
     */
    static void registerParameters()
    {}

    /*!
     * \brief Returns the name of a primary variable or an empty
     *        string if the specified primary variable index does not belong to
     *        the energy module.
     */
    static std::string primaryVarName(int pvIdx)
    {
        if (pvIdx == temperatureIdx)
            return "temperature";
        return "";
    }

    /*!
     * \brief Returns the name of an equation or an empty
     *        string if the specified equation index does not belong to
     *        the energy module.
     */
    static std::string eqName(int eqIdx)
    {
        if (eqIdx == energyEqIdx)
            return "energy";
        return "";
    }

    /*!
     * \brief Returns the relative weight of a primary variable for
     *        calculating relative errors.
     */
    static Scalar primaryVarWeight(const Model &model, int globalDofIdx, int pvIdx)
    {
        if (pvIdx != temperatureIdx)
            return -1;

        // make the weight of the temperature primary variable inversly proportional to its value
        return std::max(1.0/1000, 1.0/model.solution(/*timeIdx=*/0)[globalDofIdx][temperatureIdx]);
    }

    /*!
     * \brief Returns the relative weight of a equation.
     */
    static Scalar eqWeight(const Model &model,
                           int globalDofIdx,
                           int eqIdx)
    {
        if (eqIdx != energyEqIdx)
            return -1;

        // approximate heat capacity of 1kg of air
        return 1.0 / 1.0035e3;
    }

    /*!
     * \brief Add the rate of the enthalpy flux to a rate vector.
     */
    static void setEnthalpyRate(RateVector &rateVec, const Evaluation& rate)
    { rateVec[energyEqIdx] = rate; }

    /*!
     * \brief Add the rate of the enthalpy flux to a rate vector.
     */
    static void addToEnthalpyRate(RateVector &rateVec, const Evaluation& rate)
    { rateVec[energyEqIdx] += rate; }

    /*!
     * \brief Returns the rate of the conductive heat flux for a given flux
     *        integration point.
     */
    static Evaluation heatConductionRate(const ExtensiveQuantities &extQuants)
    { return -extQuants.temperatureGradNormal() * extQuants.heatConductivity(); }

    /*!
     * \brief Given a fluid state, set the enthalpy rate which emerges
     *        from a volumetric rate.
     */
    template <class FluidState>
    static void setEnthalpyRate(RateVector &rateVec,
                                const FluidState &fluidState, int phaseIdx,
                                Scalar volume)
    {
        rateVec[energyEqIdx] =
            volume
            * fluidState.density(phaseIdx)
            * fluidState.enthalpy(phaseIdx);
    }

    /*!
     * \brief Given a fluid state, set the temperature in the primary variables
     */
    template <class FluidState>
    static void setPriVarTemperatures(PrimaryVariables &priVars,
                                      const FluidState &fs)
    {
        priVars[temperatureIdx] = Toolbox::value(fs.temperature(/*phaseIdx=*/0));
#ifndef NDEBUG
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            assert(std::abs(Toolbox::value(fs.temperature(/*phaseIdx=*/0))
                            - Toolbox::value(fs.temperature(phaseIdx))) < 1e-30);
        }
#endif
    }

    /*!
     * \brief Add the energy storage term for a fluid phase to an equation
     * vector
     */
    template <class LhsEval>
    static void addPhaseStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                const IntensiveQuantities &intQuants, int phaseIdx)
    {
        const auto &fs = intQuants.fluidState();
        storage[energyEqIdx] +=
            Toolbox::template toLhs<LhsEval>(fs.density(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(fs.internalEnergy(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(fs.saturation(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(intQuants.porosity());
    }

    /*!
     * \brief Add the energy storage term for a fluid phase to an equation
     * vector
     */
    template <class Scv, class LhsEval>
    static void addFracturePhaseStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                        const IntensiveQuantities &intQuants,
                                        const Scv &scv, int phaseIdx)
    {
        const auto &fs = intQuants.fractureFluidState();
        storage[energyEqIdx] +=
            Toolbox::template toLhs<LhsEval>(fs.density(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(fs.internalEnergy(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(fs.saturation(phaseIdx))
            * Toolbox::template toLhs<LhsEval>(intQuants.fracturePorosity())
            * Toolbox::template toLhs<LhsEval>(intQuants.fractureVolume())/scv.volume();
    }

    /*!
     * \brief Add the energy storage term for a fluid phase to an equation
     * vector
     */
    template <class LhsEval>
    static void addSolidHeatStorage(Dune::FieldVector<LhsEval, numEq> &storage,
                                    const IntensiveQuantities &intQuants)
    {
        storage[energyEqIdx] +=
            Toolbox::template toLhs<LhsEval>(intQuants.heatCapacitySolid())
            * Toolbox::template toLhs<LhsEval>(intQuants.fluidState().temperature(/*phaseIdx=*/0));
    }

    /*!
     * \brief Evaluates the advective energy fluxes for a flux integration point and adds
     *        the result in the flux vector.
     *
     * This method is called by compute flux (base class)
     */
    template <class Context>
    static void addAdvectiveFlux(RateVector &flux, const Context &context, int spaceIdx, int timeIdx)
    {
        const auto &extQuants = context.extensiveQuantities(spaceIdx, timeIdx);

        // advective heat flux in all phases
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            if (!context.model().phaseIsConsidered(phaseIdx))
                continue;

            // intensive quantities of the upstream and the downstream DOFs
            const IntensiveQuantities &up =
                context.intensiveQuantities(extQuants.upstreamIndex(phaseIdx), timeIdx);

            flux[energyEqIdx] +=
                extQuants.volumeFlux(phaseIdx)
                * up.fluidState().enthalpy(phaseIdx)
                * up.fluidState().density(phaseIdx);
        }
    }

    /*!
     * \brief Evaluates the advective energy fluxes over a fracture which should be
     *        attributed to a face of a subcontrol volume and adds the result in the flux
     *        vector.
     */
    template <class Context>
    static void handleFractureFlux(RateVector &flux, const Context &context,
                                   int spaceIdx, int timeIdx)
    {
        const auto &scvf = context.stencil(timeIdx).interiorFace(spaceIdx);
        const auto &extQuants = context.extensiveQuantities(spaceIdx, timeIdx);

        // reduce the heat flux in the matrix by the half the width
        // occupied by the fracture
        flux[energyEqIdx] *=
            1 - extQuants.fractureWidth()/(2*scvf.area());

        // advective heat flux in all phases
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            if (!context.model().phaseIsConsidered(phaseIdx))
                continue;

            // intensive quantities of the upstream and the downstream DOFs
            const IntensiveQuantities &up =
                context.intensiveQuantities(extQuants.upstreamIndex(phaseIdx), timeIdx);

            flux[energyEqIdx] +=
                extQuants.fractureVolumeFlux(phaseIdx)
                * up.fluidState().enthalpy(phaseIdx)
                * up.fluidState().density(phaseIdx);
        }
    }

    /*!
     * \brief Adds the diffusive heat flux to the flux vector over
     *        the face of a sub-control volume.
     *
     * This method is called by compute flux (base class)
     */
    template <class Context>
    static void addDiffusiveFlux(RateVector &flux, const Context &context,
                                 int spaceIdx, int timeIdx)
    {
        const auto &extQuants = context.extensiveQuantities(spaceIdx, timeIdx);

        // diffusive heat flux
        flux[energyEqIdx] +=
            - extQuants.temperatureGradNormal()
            * extQuants.heatConductivity();
    }
};

/*!
 * \ingroup Energy
 * \class Ewoms::EnergyIndices
 *
 * \brief Provides the indices required for the energy equation.
 */
template <int PVOffset, bool enableEnergy>
struct EnergyIndices;

/*!
 * \copydoc Ewoms::EnergyIndices
 */
template <int PVOffset>
struct EnergyIndices<PVOffset, /*enableEnergy=*/false>
{
protected:
    enum { numEq_ = 0 };
};

/*!
 * \copydoc Ewoms::EnergyIndices
 */
template <int PVOffset>
struct EnergyIndices<PVOffset, /*enableEnergy=*/true>
{
    //! The index of the primary variable representing temperature
    enum { temperatureIdx = PVOffset };

    //! The index of the equation representing the conservation of energy
    enum { energyEqIdx = PVOffset };

protected:
    enum { numEq_ = 1 };
};

/*!
 * \ingroup Energy
 * \class Ewoms::EnergyIntensiveQuantities
 *
 * \brief Provides the volumetric quantities required for the energy equation.
 */
template <class TypeTag, bool enableEnergy>
class EnergyIntensiveQuantities;

/*!
 * \copydoc Ewoms::EnergyIntensiveQuantities
 */
template <class TypeTag>
class EnergyIntensiveQuantities<TypeTag, /*enableEnergy=*/false>
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, Evaluation) Evaluation;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef typename FluidSystem::ParameterCache ParameterCache;

    typedef Opm::MathToolbox<Evaluation> Toolbox;

public:
    /*!
     * \brief Returns the total heat capacity \f$\mathrm{[J/(K*m^3]}\f$ of the
     * rock matrix in
     *        the sub-control volume.
     */
    Evaluation heatCapacitySolid() const
    {
        OPM_THROW(std::logic_error, "Method heatCapacitySolid() does not make "
                                    "sense for isothermal models");
    }

    /*!
     * \brief Returns the total conductivity capacity
     *        \f$\mathrm{[W/m^2 / (K/m)]}\f$ of the rock matrix in the
     *        sub-control volume.
     */
    Evaluation heatConductivity() const
    {
        OPM_THROW(std::logic_error, "Method heatConductivity() does not make "
                                    "sense for isothermal models");
    }

protected:
    /*!
     * \brief Update the temperatures of the fluids of a fluid state.
     */
    template <class FluidState, class Context>
    static void updateTemperatures_(FluidState &fluidState,
                                    const Context &context, int spaceIdx,
                                    int timeIdx)
    {
        Scalar T = context.problem().temperature(context, spaceIdx, timeIdx);
        fluidState.setTemperature(Toolbox::createConstant(T));
    }

    /*!
     * \brief Update the quantities required to calculate
     *        energy fluxes.
     */
    template <class FluidState>
    void update_(FluidState &fs,
                 ParameterCache &paramCache,
                 const ElementContext &elemCtx,
                 int dofIdx,
                 int timeIdx)
    { }
};

/*!
 * \copydoc Ewoms::EnergyIntensiveQuantities
 */
template <class TypeTag>
class EnergyIntensiveQuantities<TypeTag, /*enableEnergy=*/true>
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, Evaluation) Evaluation;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;
    typedef typename GET_PROP_TYPE(TypeTag, FluidSystem) FluidSystem;
    typedef typename GET_PROP_TYPE(TypeTag, HeatConductionLaw) HeatConductionLaw;
    typedef typename GET_PROP_TYPE(TypeTag, Indices) Indices;
    typedef typename FluidSystem::ParameterCache ParameterCache;

    enum { numPhases = FluidSystem::numPhases };
    enum { energyEqIdx = Indices::energyEqIdx };
    enum { temperatureIdx = Indices::temperatureIdx };

    typedef Opm::MathToolbox<Evaluation> Toolbox;

protected:
    /*!
     * \brief Update the temperatures of the fluids of a fluid state.
     */
    template <class FluidState, class Context>
    static void updateTemperatures_(FluidState &fluidState,
                                    const Context &context, int spaceIdx,
                                    int timeIdx)
    {
        const auto& priVars = context.primaryVars(spaceIdx, timeIdx);
        Evaluation val;
        if (timeIdx == 0)
            val = Toolbox::createVariable(priVars[temperatureIdx], temperatureIdx);
        else
            val = Toolbox::createConstant(priVars[temperatureIdx]);

        fluidState.setTemperature(val);
    }

    /*!
     * \brief Update the quantities required to calculate
     *        energy fluxes.
     */
    template <class FluidState>
    void update_(FluidState &fs,
                 ParameterCache &paramCache,
                 const ElementContext &elemCtx,
                 int dofIdx,
                 int timeIdx)
    {
        // set the specific enthalpies of the fluids
        for (int phaseIdx = 0; phaseIdx < numPhases; ++phaseIdx) {
            if (!elemCtx.model().phaseIsConsidered(phaseIdx))
                continue;

            fs.setEnthalpy(phaseIdx,
                           FluidSystem::enthalpy(fs, paramCache, phaseIdx));
        }

        // compute and set the heat capacity of the solid phase
        const auto &problem = elemCtx.problem();
        const auto &heatCondParams = problem.heatConductionParams(elemCtx, dofIdx, timeIdx);

        heatCapacitySolid_ = problem.heatCapacitySolid(elemCtx, dofIdx, timeIdx);
        heatConductivity_ =
            HeatConductionLaw::template heatConductivity<FluidState, Evaluation>(heatCondParams, fs);

        Valgrind::CheckDefined(heatCapacitySolid_);
        Valgrind::CheckDefined(heatConductivity_);
    }

public:
    /*!
     * \brief Returns the total heat capacity \f$\mathrm{[J/(K*m^3]}\f$ of the
     * rock matrix in
     *        the sub-control volume.
     */
    const Evaluation& heatCapacitySolid() const
    { return heatCapacitySolid_; }

    /*!
     * \brief Returns the total conductivity capacity
     *        \f$\mathrm{[W/m^2 / (K/m)]}\f$ of the rock matrix in the
     *        sub-control volume.
     */
    const Evaluation& heatConductivity() const
    { return heatConductivity_; }

private:
    Evaluation heatCapacitySolid_;
    Evaluation heatConductivity_;
};

/*!
 * \ingroup Energy
 * \class Ewoms::EnergyExtensiveQuantities
 *
 * \brief Provides the quantities required to calculate energy fluxes.
 */
template <class TypeTag, bool enableEnergy>
class EnergyExtensiveQuantities;

/*!
 * \copydoc Ewoms::EnergyExtensiveQuantities
 */
template <class TypeTag>
class EnergyExtensiveQuantities<TypeTag, /*enableEnergy=*/false>
{
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;

protected:
    /*!
     * \brief Update the quantities required to calculate
     *        energy fluxes.
     */
    void update_(const ElementContext &elemCtx, int faceIdx, int timeIdx)
    {}

    template <class Context, class FluidState>
    void updateBoundary_(const Context &context, int bfIdx, int timeIdx,
                         const FluidState &fs)
    {}

public:
    /*!
     * \brief The temperature gradient times the face normal [K m^2 / m]
     */
    Scalar temperatureGradNormal() const
    {
        OPM_THROW(std::logic_error, "Method temperatureGradNormal() does not "
                                    "make sense for isothermal models");
    }

    /*!
     * \brief The total heat conductivity at the face \f$\mathrm{[W/m^2 /
     * (K/m)]}\f$
     */
    Scalar heatConductivity() const
    {
        OPM_THROW(std::logic_error, "Method heatConductivity() does not make "
                                    "sense for isothermal models");
    }
};

/*!
 * \copydoc Ewoms::EnergyExtensiveQuantities
 */
template <class TypeTag>
class EnergyExtensiveQuantities<TypeTag, /*enableEnergy=*/true>
{
    typedef typename GET_PROP_TYPE(TypeTag, ElementContext) ElementContext;
    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, Evaluation) Evaluation;
    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;

    enum { dimWorld = GridView::dimensionworld };
    typedef Dune::FieldVector<Evaluation, dimWorld> EvalDimVector;
    typedef Dune::FieldVector<Scalar, dimWorld> DimVector;

protected:
    /*!
     * \brief Update the quantities required to calculate
     *        energy fluxes.
     */
    void update_(const ElementContext &elemCtx, int faceIdx, int timeIdx)
    {
        const auto& gradCalc = elemCtx.gradientCalculator();
        Ewoms::TemperatureCallback<TypeTag> temperatureCallback(elemCtx);

        EvalDimVector temperatureGrad;
        gradCalc.calculateGradient(temperatureGrad,
                                   elemCtx,
                                   faceIdx,
                                   temperatureCallback);

        // scalar product of temperature gradient and scvf normal
        const auto &face = elemCtx.stencil(/*timeIdx=*/0).interiorFace(faceIdx);

        temperatureGradNormal_ = 0;
        for (int dimIdx = 0; dimIdx < dimWorld; ++dimIdx)
            temperatureGradNormal_ += (face.normal()[dimIdx]*temperatureGrad[dimIdx]);

        const auto &extQuants = elemCtx.extensiveQuantities(faceIdx, timeIdx);
        const auto &intQuantsInside = elemCtx.intensiveQuantities(extQuants.interiorIndex(), timeIdx);
        const auto &intQuantsOutside = elemCtx.intensiveQuantities(extQuants.exteriorIndex(), timeIdx);

        // arithmetic mean
        heatConductivity_ =
            0.5 * (intQuantsInside.heatConductivity() + intQuantsOutside.heatConductivity());
        Valgrind::CheckDefined(heatConductivity_);
    }

    template <class Context, class FluidState>
    void updateBoundary_(const Context &context, int bfIdx, int timeIdx, const FluidState &fs)
    {
        const auto &stencil = context.stencil(timeIdx);
        const auto &face = stencil.boundaryFace(bfIdx);

        const auto &elemCtx = context.elementContext();
        int insideScvIdx = face.interiorIndex();
        const auto &insideScv = elemCtx.stencil(timeIdx).subControlVolume(insideScvIdx);

        const auto &intQuantsInside = elemCtx.intensiveQuantities(insideScvIdx, timeIdx);
        const auto &fsInside = intQuantsInside.fluidState();

        // distance between the center of the SCV and center of the boundary face
        DimVector distVec = face.integrationPos();
        distVec -= insideScv.geometry().center();

        Scalar tmp = 0;
        for (int dimIdx = 0; dimIdx < dimWorld; ++dimIdx)
            tmp += distVec[dimIdx] * face.normal()[dimIdx];
        Scalar dist = tmp;

        // if the following assertation triggers, the center of the
        // center of the interior SCV was not inside the element!
        assert(dist > 0);

        // calculate the temperature gradient using two-point gradient
        // appoximation
        temperatureGradNormal_ =
            (fs.temperature(/*phaseIdx=*/0) - fsInside.temperature(/*phaseIdx=*/0)) / dist;

        // take the value for heat conductivity from the interior finite volume
        heatConductivity_ = intQuantsInside.heatConductivity();
    }

public:
    /*!
     * \brief The temperature gradient times the face normal [K m^2 / m]
     */
    const Evaluation& temperatureGradNormal() const
    { return temperatureGradNormal_; }

    /*!
     * \brief The total heat conductivity at the face \f$\mathrm{[W/m^2 /
     * (K/m)]}\f$
     */
    const Evaluation& heatConductivity() const
    { return heatConductivity_; }

private:
    Evaluation temperatureGradNormal_;
    Evaluation heatConductivity_;
};

} // namespace Ewoms

#endif
