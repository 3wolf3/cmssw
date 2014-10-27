#ifndef OuterTrackerMCTruth_OuterTrackerMCTruth_h
#define OuterTrackerMCTruth_OuterTrackerMCTruth_h

#include <vector>
#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include <DQMServices/Core/interface/DQMStore.h>
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DQM/SiStripCommon/interface/TkHistoMap.h"

class DQMStore;

class OuterTrackerMCTruth : public edm::EDAnalyzer {

public:
  explicit OuterTrackerMCTruth(const edm::ParameterSet&);
  ~OuterTrackerMCTruth();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  //virtual void beginJob() ;
  virtual void endJob() ;
  virtual void beginRun(const edm::Run&, const edm::EventSetup&);
 
  
	// TrackingParticle and TrackingVertex
	MonitorElement* TPart_Pt = 0;
	MonitorElement* TPart_Eta_Pt10 = 0;
	MonitorElement* TPart_Phi_Pt10 = 0;
	MonitorElement* SimVtx_XY = 0;
	MonitorElement* SimVtx_RZ = 0;
	// CW vs. TPart Eta
	MonitorElement* TPart_Eta_INormalization = 0;
	MonitorElement* TPart_Eta_ICW_1 = 0;
	MonitorElement* TPart_Eta_ICW_2 = 0;
	MonitorElement* TPart_Eta_ICW_3 = 0;
	MonitorElement* TPart_Eta_ONormalization = 0;
	MonitorElement* TPart_Eta_OCW_1 = 0;
	MonitorElement* TPart_Eta_OCW_2 = 0;
	MonitorElement* TPart_Eta_OCW_3 = 0;
	// Stub in PS/2S module vs. TPart Eta
	MonitorElement* TPart_Eta_Pt10_Normalization = 0;
	MonitorElement* TPart_Eta_Pt10_NumPS = 0;
	MonitorElement* TPart_Eta_Pt10_Num2S = 0;
	

 private:
  DQMStore* dqmStore_;
  edm::ParameterSet conf_;

  std::string topFolderName_;
  
};
#endif
