#include "DetectorDescription/Core/interface/DDFilteredView.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "Geometry/MTDNumberingBuilder/interface/GeometricTimingDet.h"
#include "Geometry/MTDNumberingBuilder/plugins/CmsMTDLevelBuilder.h"
#include "Geometry/MTDNumberingBuilder/plugins/ExtractStringFromDDD.h"

bool subDetByType(const GeometricTimingDet* a, const GeometricTimingDet* b)
{
    // it relies on the fact that the GeometricTimingDet::GDEnumType
    // enumerators used to identify the subdetectors in the upgrade geometries
    // are equal to the ones of the present detector + n*100
    return a->type() < b->type();
}

// NP** Phase2 BarrelEndcap
bool phiSortNP(const GeometricTimingDet* a, const GeometricTimingDet* b)
{
    if (fabs(a->translation().rho() - b->translation().rho()) < 0.01
        && (fabs(a->translation().phi() - b->translation().phi()) < 0.01
               || fabs(a->translation().phi() - b->translation().phi()) > 6.27)
        && a->translation().z() * b->translation().z() > 0.0) {
        return (fabs(a->translation().z()) < fabs(b->translation().z()));
    } else
        return false;
}

bool isLessZ(const GeometricTimingDet* a, const GeometricTimingDet* b)
{
    // NP** change for Phase 2 Tracker
    if (a->translation().z() == b->translation().z()) {
        return a->translation().rho() < b->translation().rho();
    } else {
        // Original version
        return a->translation().z() < b->translation().z();
    }
}

bool isLessModZ(const GeometricTimingDet* a, const GeometricTimingDet* b)
{
    return fabs(a->translation().z()) < fabs(b->translation().z());
}

double getPhi(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    double phi = a->phi();
    return (phi >= 0 ? phi : phi + 2 * pi);
}

double getPhiModule(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    std::vector<const GeometricTimingDet*> const& comp = a->components().back()->components();
    float phi = 0.;
    bool sum = true;

    for (auto i : comp) {
        if (fabs(i->phi()) > pi / 2.) {
            sum = false;
            break;
        }
    }

    if (sum) {
        for (auto i : comp) {
            phi += i->phi();
        }

        double temp = phi / float(comp.size()) < 0. ? 2 * pi + phi / float(comp.size()) : phi / float(comp.size());
        return temp;

    } else {
        for (auto i : comp) {
            double phi1 = i->phi() >= 0 ? i->phi() : i->phi() + 2 * pi;
            phi += phi1;
        }

        double com = comp.front()->phi() >= 0 ? comp.front()->phi() : 2 * pi + comp.front()->phi();
        double temp
            = fabs(phi / float(comp.size()) - com) > 2. ? pi - phi / float(comp.size()) : phi / float(comp.size());
        temp = temp >= 0 ? temp : 2 * pi + temp;
        return temp;
    }
}

double getPhiGluedModule(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    std::vector<const GeometricTimingDet*> comp;
    a->deepComponents(comp);
    float phi = 0.;
    bool sum = true;

    for (auto& i : comp) {
        if (fabs(i->phi()) > pi / 2.) {
            sum = false;
            break;
        }
    }

    if (sum) {
        for (auto& i : comp) {
            phi += i->phi();
        }

        double temp = phi / float(comp.size()) < 0. ? 2 * pi + phi / float(comp.size()) : phi / float(comp.size());
        return temp;

    } else {
        for (auto& i : comp) {
            double phi1 = i->phi() >= 0 ? i->phi() : i->translation().phi() + 2 * pi;
            phi += phi1;
        }

        double com = comp.front()->phi() >= 0 ? comp.front()->phi() : 2 * pi + comp.front()->phi();
        double temp
            = fabs(phi / float(comp.size()) - com) > 2. ? pi - phi / float(comp.size()) : phi / float(comp.size());
        temp = temp >= 0 ? temp : 2 * pi + temp;
        return temp;
    }
}

double getPhiMirror(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    double phi = a->phi();
    phi = (phi >= 0 ? phi : phi + 2 * pi); // (-pi,pi] --> [0,2pi)
    return ((pi - phi) >= 0 ? (pi - phi) : (pi - phi) + 2 * pi); // (-pi,pi] --> [0,2pi)
}

double getPhiModuleMirror(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    double phi = getPhiModule(a); // [0,2pi)
    phi = (phi <= pi ? phi : phi - 2 * pi); // (-pi,pi]
    return (pi - phi);
}

double getPhiGluedModuleMirror(const GeometricTimingDet* a)
{
    const double pi = 3.141592653592;
    double phi = getPhiGluedModule(a); // [0,2pi)
    phi = (phi <= pi ? phi : phi - 2 * pi); // (-pi,pi]
    return (pi - phi);
}

bool isLessRModule(const GeometricTimingDet* a, const GeometricTimingDet* b)
{
    return a->deepComponents().front()->rho() < b->deepComponents().front()->rho();
}

bool isLessR(const GeometricTimingDet* a, const GeometricTimingDet* b) { return a->rho() < b->rho(); }

void CmsMTDLevelBuilder::build(DDFilteredView& fv, GeometricTimingDet* tracker, std::string attribute)
{

    LogTrace("GeometricTimingDetBuilding")
        << std::string(3 * fv.history().size(), '-') << "+ " << ExtractStringFromDDD::getString(attribute, &fv) << " "
        << tracker->type() << " " << tracker->name() << std::endl;

    bool doLayers = fv.firstChild(); // descend to the next Layer

    while (doLayers) {
        buildComponent(fv, tracker, attribute);
        doLayers = fv.nextSibling(); // go to the next adjacent thingy
    }

    fv.parent();

    sortNS(fv, tracker);
}
