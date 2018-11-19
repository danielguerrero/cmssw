#include "DD4hep/DetFactoryHelper.h"
#include "DetectorDescription/DDCMS/interface/DDPlugins.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

//#define EDM_ML_DEBUG

static long  algorithm(dd4hep::Detector& /* description */,
                       cms::DDParsingContext& ctxt,
                       xml_h e,
                       dd4hep::SensitiveDetector& /* sens */) {

  cms::DDNamespace      ns(ctxt,e,true);
  cms::DDAlgoArguments  args(ctxt, e);
  // Header section of original DDHCalAngular.h
  int            n           = args.value<int>("n");
  int            startCopyNo = args.value<int>("startCopyNo");
  int            incrCopyNo  = args.value<int>("incrCopyNo");
  double         rangeAngle  = args.value<double>("rangeAngle"); //Angular range
  double         startAngle  = args.value<double>("startAngle"); //Start anle
  double         shiftX      = args.value<double>("shiftX");     //x Shift
  double         shiftY      = args.value<double>("shiftY");     //y Shift
  double         zoffset     = args.value<double>("zoffset");    //z offset
  dd4hep::Volume mother      = ns.volume(args.parentName());
  dd4hep::Volume child       = ns.volume(args.value<std::string>("ChildName"));
  // Increment
  double         dphi        = rangeAngle/n;
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HCalGeom") << "DDHCalAngular: Parameters for positioning::"
			       << " n " << n << " Start, Range, Delta " 
			       << ConvertTo( startAngle, deg ) << " " 
			       << ConvertTo( rangeAngle, deg ) << " " 
			       << ConvertTo( dphi, deg )
			       << " Shift " << shiftX << ":" << shiftY
			       << "\n Parent " << mother.name() 
			       << "\tChild " << child.name() 
			       << " NameSpace " << ns.name();
#endif
  double theta  = 90._deg;
  int    copy   = startCopyNo;
  double phi    = startAngle;
  for (int ii=0; ii<n; ++ii) {
    double phix   = ConvertTo( phi, deg );
    double phiy   = phix + 90._deg;
    int    iphi   = (phix > 0) ? int(phix+0.1) : int(phix-0.1);
    if (iphi >= 360) iphi -= 360;
    phix = iphi;
    dd4hep::Rotation3D rotation;
    if (iphi != 0) {
#ifdef EDM_ML_DEBUG
      edm::LogVerbatim("HCalGeom") << "DDHCalAngular::Creating a rotation:"
				   << "\t90., " << phix << ", 90.," 
				   << phiy <<", 0, 0";
#endif
      rotation = cms::makeRotation3D(theta, phix, theta, phiy, 0., 0.);
    }
	
    double xpos = shiftX*cos(phi) - shiftY*sin(phi);
    double ypos = shiftX*sin(phi) + shiftY*cos(phi);
    dd4hep::Position tran(xpos, ypos, zoffset);
    mother.placeVolume(child, copy, dd4hep::Transform3D(rotation,tran));
#ifdef EDM_ML_DEBUG
    edm::LogVerbatim("HCalGeom") << "DDHCalAngular:: " << child.name() 
				 << " number " << copy << " positioned in " 
				 << mother.name() << " at " << tran  
				 << " with " << rotation;
#endif
    copy += incrCopyNo;
    phi  += dphi;
  }
  return 1;
}

// first argument is the type from the xml file
DECLARE_DDCMS_DETELEMENT(DDCMS_Hcal_DDHCalAngular,algorithm)
