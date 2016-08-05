#include <iostream>

#include "HLTrigger/HLTanalyzers/interface/HLTInfo.h"
#include "HLTrigger/HLTanalyzers/interface/HLTMCtruth.h"
#include "HLTrigger/HLTanalyzers/interface/HLTMuon.h"
#include "HLTrigger/HLTanalyzers/interface/EventHeader.h"
#include "HLTrigger/HLTanalyzers/interface/RECOVertex.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/Registry.h"

#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "Geometry/CaloEventSetup/interface/CaloTopologyRecord.h"  

#include "CondFormats/DataRecord/interface/L1CaloGeometryRecord.h"  

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutRecord.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerReadoutSetupFwd.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMapRecord.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMapFwd.h"
#include "DataFormats/L1GlobalTrigger/interface/L1GlobalTriggerObjectMap.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "DataFormats/HLTReco/interface/TriggerFilterObjectWithRefs.h"

/** \class HLTAnalyzer
  *  
  * $Date: November 2006
  * $Revision: 
  * \author P. Bargassa - Rice U.
  */

class HLTAnalyzer : public edm::EDAnalyzer {
public:
  explicit HLTAnalyzer(edm::ParameterSet const& conf);
  virtual void analyze(edm::Event const& e, edm::EventSetup const& iSetup);
  virtual void beginRun(const edm::Run& , const edm::EventSetup& );
  virtual void endJob();

  //  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions); 

  // Analysis tree to be filled
  TTree *HltTree;

private:
  // variables persistent across events should be declared here.
  //
  ///Default analyses

  EventHeader evt_header_;
  HLTMuon     muon_analysis_;
  HLTMCtruth  mct_analysis_;
  HLTInfo     hlt_analysis_;
  RECOVertex  vrt_analysisHLT_;
  RECOVertex  vrt_analysisOffline0_;

  int firstLumi_, lastLumi_;
  double xSection_, filterEff_, treeWeight;

  //
  // All tokens needed to access products in the event
  //
  std::vector< edm::EDGetTokenT<trigger::TriggerFilterObjectWithRefs> >  muonFilterTokens_;

  edm::EDGetTokenT<reco::BeamSpot>                       BSProducerToken_;
  edm::EDGetTokenT<reco::CandidateView>                  mctruthToken_;
  edm::EDGetTokenT<GenEventInfoProduct>                  genEventInfoToken_;
  edm::EDGetTokenT<std::vector<SimTrack> >               simTracksToken_;
  edm::EDGetTokenT<std::vector<SimVertex> >              simVerticesToken_;
  edm::EDGetTokenT<reco::MuonCollection>                 muonToken_;
  edm::EDGetTokenT<edm::TriggerResults>                  hltresultsToken_;
  edm::EDGetTokenT<l1extra::L1EmParticleCollection>      l1extraemiToken_, l1extraemnToken_;
  edm::EDGetTokenT<l1extra::L1MuonParticleCollection>    l1extramuToken_;
  edm::EDGetTokenT<l1extra::L1JetParticleCollection>     l1extrajetcToken_, l1extrajetfToken_, l1extrajetToken_, l1extrataujetToken_;
  edm::EDGetTokenT<l1extra::L1EtMissParticleCollection>  l1extrametToken_,l1extramhtToken_;
  edm::EDGetTokenT<L1GlobalTriggerReadoutRecord>         gtReadoutRecordToken_;
  edm::EDGetTokenT< L1GctHFBitCountsCollection >         gctBitCountsToken_;
  edm::EDGetTokenT< L1GctHFRingEtSumsCollection >        gctRingSumsToken_;
    
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> MuCandTag2Token_, MuCandTag3Token_;
  edm::EDGetTokenT<std::vector<reco::Track>> L3TkTracksFromL2OIStateToken_, L3TkTracksFromL2OIHitToken_;
              
    // Reco vertex collection
  edm::EDGetTokenT<reco::VertexCollection> VertexHLTToken_;
  edm::EDGetTokenT<reco::VertexCollection> VertexOffline0Token_;

  //
  // All input tags
  //
  std::vector< edm::InputTag > muonFilterCollections_;

  edm::InputTag BSProducer_;

  edm::InputTag hltresults_,genEventInfo_;
  edm::InputTag muon_;
  std::string l1extramc_, l1extramu_;
  edm::InputTag m_l1extramu;
  edm::InputTag m_l1extraemi;
  edm::InputTag m_l1extraemn;
  edm::InputTag m_l1extrajetc;
  edm::InputTag m_l1extrajetf;
  edm::InputTag m_l1extrajet;
  edm::InputTag m_l1extrataujet;
  edm::InputTag m_l1extramet;
  edm::InputTag m_l1extramht;

  edm::InputTag particleMapSource_,mctruth_,simhits_; 
  edm::InputTag gtReadoutRecord_,gtObjectMap_; 
  edm::InputTag gctBitCounts_,gctRingSums_;

  edm::InputTag MuCandTag2_,MuCandTag3_;
  edm::InputTag L3TkTracksFromL2OIStateTag_, L3TkTracksFromL2OIHitTag_;
  
  // Reco vertex collection
  edm::InputTag VertexTagHLT_;
  edm::InputTag VertexTagOffline0_;

  int errCnt;
  static int errMax() { return 5; }

  std::string _HistName; // Name of histogram file
  double _EtaMin,_EtaMax;
    double _MinPtChargedHadrons, _MinPtGammas;
  TFile* m_file; // pointer to Histogram file

};
