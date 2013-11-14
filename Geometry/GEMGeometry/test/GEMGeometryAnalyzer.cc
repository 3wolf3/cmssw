
/** Derived from DTGeometryAnalyzer by Nicola Amapane
 *
 *  \author M. Maggi - INFN Bari
 */

#include <memory>
#include <fstream>
#include <FWCore/Framework/interface/Frameworkfwd.h>

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"

#include "DataFormats/Math/interface/deltaPhi.h"

#include <string>
#include <cmath>
#include <vector>
#include <iomanip>
#include <set>

class GEMGeometryAnalyzer : public edm::EDAnalyzer {

public: 
  GEMGeometryAnalyzer( const edm::ParameterSet& pset);

  ~GEMGeometryAnalyzer();

  virtual void analyze( const edm::Event&, const edm::EventSetup& );
  
  const std::string& myName() { return myName_;}
  
private: 

  const int dashedLineWidth_;
  const std::string dashedLine_;
  const std::string myName_;
  std::ofstream ofos;
};

GEMGeometryAnalyzer::GEMGeometryAnalyzer( const edm::ParameterSet& /*iConfig*/ )
  : dashedLineWidth_(104), dashedLine_( std::string(dashedLineWidth_, '-') ), 
    myName_( "GEMGeometryAnalyzer" ) 
{ 
  ofos.open("MytestOutput.out"); 
  ofos <<"======================== Opening output file"<< std::endl;
}


GEMGeometryAnalyzer::~GEMGeometryAnalyzer() 
{
  ofos.close();
  ofos <<"======================== Closing output file"<< std::endl;
}

void
GEMGeometryAnalyzer::analyze( const edm::Event& /*iEvent*/, const edm::EventSetup& iSetup )
{
  edm::ESHandle<GEMGeometry> pDD;
  iSetup.get<MuonGeometryRecord>().get(pDD);     
  
  ofos << myName() << ": Analyzer..." << std::endl;
  ofos << "start " << dashedLine_ << std::endl;

  ofos << " Geometry node for GEMGeom is  " << &(*pDD) << std::endl;   
  ofos << " detTypes       \t"<<pDD->detTypes().size() << std::endl;
  ofos << " GeomDetUnit       \t"<<pDD->detUnits().size() << std::endl;
  ofos << " GeomDet           \t"<<pDD->dets().size() << std::endl;
  ofos << " GeomDetUnit DetIds\t"<<pDD->detUnitIds().size() << std::endl;
  ofos << " eta partitions \t"<<pDD->etaPartitions().size() << std::endl;
  ofos << " chambers       \t"<<pDD->chambers().size() << std::endl;
  ofos << " no. eta partitions \t"<< pDD->etaPartitions().size()/pDD->chambers().size() << std::endl;

  //ofos << " super chambers  "<<pDD->superChambers().size() << std::endl;

  // checking uniqueness of roll detIds 
  bool flagNonUniqueRollID = false;
  bool flagNonUniqueRollRawID = false;
  int nstrips = 0;
  int npads = 0;
  for (auto roll1 : pDD->etaPartitions()){
    nstrips += roll1->nstrips();
    npads += roll1->npads();
    for (auto roll2 : pDD->etaPartitions()){
      if (roll1 != roll2){
	if (roll1->id() == roll2->id()) flagNonUniqueRollID = true;
	if (roll1->id().rawId() == roll2->id().rawId()) flagNonUniqueRollRawID = true;
      }
    }
  }
  ofos << " total number of strips\t"<<nstrips << std::endl;
  ofos << " total number of pads  \t"<<npads << std::endl;
  ofos << " flagNonUniqueRollID   \t"<<flagNonUniqueRollID << std::endl;
  ofos << " flagNonUniqueRollRawID\t"<<flagNonUniqueRollRawID << std::endl;

  // checking uniqueness of chamber detIds
  bool flagNonUniqueChID = false;
  bool flagNonUniqueChRawID = false;
  for (auto ch1 : pDD->chambers()){
    for (auto ch2 : pDD->chambers()){
      if (ch1 != ch2){
	if (ch1->id() == ch2->id()) flagNonUniqueChID = true;
	if (ch1->id().rawId() == ch2->id().rawId()) flagNonUniqueChRawID = true;
      }
    }
  }
  ofos << " flagNonUniqueChID     \t"<<flagNonUniqueChID << std::endl;
  ofos << " flagNonUniqueChRawID  \t"<<flagNonUniqueChRawID << std::endl;

  // checking the number of strips and pads

  ofos << myName() << ": Begin iteration over geometry..." << std::endl;
  ofos << "iter " << dashedLine_ << std::endl;
  
  //----------------------- Global GEMGeometry TEST -------------------------------------------------------
  ofos << myName() << "Begin GEMGeometry structure TEST" << std::endl;
  
  int j = 1;


  // checking the dimensions of each partition & chamber
  for (auto ch : pDD->chambers()){
    GEMDetId chId(ch->id());
    int nRolls(ch->nEtaPartitions());
    ofos << "  GEMChamber " << j << ", GEMDetId = " << chId.rawId() << ", " << chId << " has " << nRolls << " eta partitions." << std::endl;

    int k = 1;
    auto& rolls(ch->etaPartitions());
    
    /*
     * possible checklist for an eta partition:
     *   base_bottom, base_top, height, strips, pads
     *   cx, cy, cz, ceta, cphi
     *   tx, ty, tz, teta, tphi
     *   bx, by, bz, beta, bphi
     *   pitch center, pitch bottom, pitch top
     *   deta, dphi
     *   gap thickness
     *   sum of all dx + gap = chamber height
     */      

    for (auto roll : rolls){
      GEMDetId rId(roll->id());
      ofos<<"    GEMEtaPartition " << k << ", GEMDetId = " << rId.rawId() << ", " << rId << std::endl;
      
      const BoundPlane& bSurface(roll->surface());
      const StripTopology* topology(&(roll->specificTopology()));
      
      // base_bottom, base_top, height, strips, pads (all half length)
      auto& parameters(roll->specs()->parameters());
      float bottomEdge(parameters[0]);
      float topEdge(parameters[1]);
      float height(parameters[2]);
      float nStrips(parameters[3]);
      float nPads(parameters[4]);
      
      LocalPoint  lCentre( 0., 0., 0. );
      GlobalPoint gCentre(bSurface.toGlobal(lCentre));
      
      LocalPoint  lTop( 0., height, 0.);
      GlobalPoint gTop(bSurface.toGlobal(lTop));
      
      LocalPoint  lBottom( 0., -height, 0.);
      GlobalPoint gBottom(bSurface.toGlobal(lBottom));
      
      //   gx, gy, gz, geta, gphi (center)
      double cx(gCentre.x());
      double cy(gCentre.y());
      double cz(gCentre.z());
      double ceta(gCentre.eta());
      int cphi(static_cast<int>(gCentre.phi().degrees()));
      if (cphi < 0) cphi += 360;
      
      double tx(gTop.x());
      double ty(gTop.y());
      double tz(gTop.z());
      double teta(gTop.eta());
      int tphi(static_cast<int>(gTop.phi().degrees()));
      if (tphi < 0) tphi += 360;
      
      double bx(gBottom.x());
      double by(gBottom.y());
      double bz(gBottom.z());
      double beta(gBottom.eta());
      int bphi(static_cast<int>(gBottom.phi().degrees()));
      if (bphi < 0) bphi += 360;
      
      // pitch bottom, pitch top, pitch centre
      float pitch(roll->pitch());
      float topPitch(roll->localPitch(lTop));
      float bottomPitch(roll->localPitch(lBottom));
      
      // Type - should be GHA[1-nRolls]
      std::string type(roll->type().name());
      
      // print info about edges
      LocalPoint lEdge1(topology->localPosition(0.));
      LocalPoint lEdgeN(topology->localPosition((float)nStrips));
      
      double cstrip1(roll->toGlobal(lEdge1).phi().degrees());
      double cstripN(roll->toGlobal(lEdgeN).phi().degrees());
      double dphi(cstripN - cstrip1);
      if (dphi < 0.) dphi += 360.;
      double deta(abs(beta - teta));
      ofos << "    \tType: " << type << std::endl
	   << "    \tDimensions[cm]: b = " << bottomEdge << ", B = " << topEdge << ", h  = " << height << std::endl
	   << "    \tnStrips = " << nStrips << ", nPads =  " << nPads << std::endl
	   << "    \tcenter(x,y,z) = " << cx << " " << cy << " " << cz << ", center(eta,phi) = " << ceta << " " << cphi << std::endl
	   << "    \ttop(x,y,z) = " << tx << " " << ty << " " << tz << ", top(eta,phi) = " << teta << " " << tphi << std::endl
	   << "    \tbottom(x,y,z) = " << bx << " " << by << " " << bz << ", bottom(eta,phi) = " << beta << " " << bphi << std::endl
	   << "    \tpith (top,center,bottom) = " << topPitch << " " << pitch << " " << bottomPitch << ", dEta = " << deta << ", dPhi = " << dphi << std::endl;
      
      ++k;
    }
    ++j;
  }

  ofos << dashedLine_ << " end" << std::endl;
}

//define this as a plug-in
#include <FWCore/Framework/interface/MakerMacros.h>
DEFINE_FWK_MODULE(GEMGeometryAnalyzer);
