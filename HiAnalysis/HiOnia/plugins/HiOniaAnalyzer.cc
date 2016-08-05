// -*- C++ -*-
//
// Package:    HiOniaAnalyzer
// Class:      HiOniaAnalyzer
// 
/**\class HiOniaAnalyzer HiOniaAnalyzer.cc UserCode/tdahms/HiAnalysis/HiOnia/plugins/HiOniaAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/


// system include files
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

#include <TTree.h>
#include <TLorentzVector.h>
#include <TVector2.h>
#include <TClonesArray.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "DataFormats/HeavyIonEvent/interface/Centrality.h"
#include "DataFormats/HeavyIonEvent/interface/EvtPlane.h"

#include "HiAnalysis/HiOnia/interface/MyCommonHistoManager.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//
// class declaration
//

class HiOniaAnalyzer : public edm::EDAnalyzer {
public:
  explicit HiOniaAnalyzer(const edm::ParameterSet&);
  ~HiOniaAnalyzer();
  
  
private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;
  
  
  void InitEvent();
  void InitTree();

  void makeCuts(int sign) ;
  bool checkCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1,  const pat::Muon* muon2, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*)); 


  reco::GenParticleRef findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG);
  void fillGenInfo();
  bool isAbHadron(int pdgID);
  bool isAMixedbHadron(int pdgID, int momPdgID);
  reco::GenParticleRef findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG);
  std::pair<int, std::pair<float, float> >  findGenMCInfo(const reco::GenParticle *genJpsi);

  void fillRecoMuons(int theCentralityBin);
  bool isMuonInAccept(const pat::Muon* aMuon, std::string muonType);

  bool isSoftMuon(const pat::Muon* aMuon);
  bool isTightMuon(const pat::Muon* aMuon);

  void fillRecoTracks();

  pair< unsigned int, const pat::CompositeCandidate* > theBestQQ(int sign);
  double CorrectMass(const reco::Muon& mu1,const reco::Muon& mu2, int mode);

  bool selGlobalMuon(const pat::Muon* aMuon);
  bool selTrackerMuon(const pat::Muon* aMuon);

  void fillRecoHistos(int lastSign);
  void fillRecoJpsi(int iSign, int count, std::string trigName, std::string centName);
  void fillHistosAndDS(unsigned int theCat, const pat::CompositeCandidate* aJpsiCand);

  void fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits);
  void fillTreeJpsi(int iSign, int count);

  void checkTriggers(const pat::CompositeCandidate* aJpsiCand);
  void hltReport(const edm::Event &iEvent ,const edm::EventSetup& iSetup);

  void beginRun(const edm::Run &, const edm::EventSetup &);  

  TLorentzVector lorentzMomentum(const reco::Candidate::LorentzVector& p);
  int muonIDmask(const pat::Muon* muon);

  // ----------member data ---------------------------
  enum StatBins {
    BIN_nEvents = 0
  };

  enum dimuonCategories {
    GlbTrk_GlbTrk = 0,
    Glb_Glb       = 1,
    Trk_Trk       = 2
  };

  enum muonCategories {
    GlbTrk        = 0,
    Trk           = 1,
    Glb           = 2
  };

  std::vector<std::string> theRegions;
  std::vector<std::string> theCentralities;
  std::vector<std::string> theTriggerNames;
  std::vector<std::string> theSign;

  HLTConfigProvider hltConfig;
  bool hltConfigInit;

  float etaMin;
  float etaMax;

  // TFileService
  edm::Service<TFileService> fs;

  // TFile
  TFile* fOut;

  // TTree
  TTree* myTree;

  TClonesArray* Reco_mu_4mom;
  TClonesArray* Reco_QQ_4mom;
  TClonesArray* Reco_QQ_vtx;
  TClonesArray* Reco_QQ_mupl_4mom;
  TClonesArray* Reco_QQ_mumi_4mom;
  TClonesArray* Reco_trk_4mom;
  TClonesArray* Reco_trk_vtx;

  TClonesArray* Gen_mu_4mom;
  TClonesArray* Gen_QQ_4mom;
  TClonesArray* Gen_QQ_mupl_4mom;
  TClonesArray* Gen_QQ_mumi_4mom;

  static const int Max_QQ_size = 10000;
  static const int Max_mu_size = 10000;
  static const int Max_trk_size = 10000;

  int Gen_QQ_size; // number of generated Onia
  int Gen_QQ_type[Max_QQ_size]; // Onia type: prompt, non-prompt, unmatched
  float Gen_QQ_ctau[Max_QQ_size];    // ctau: flight time
  float Gen_QQ_ctau3D[Max_QQ_size];    // ctau3D: 3D flight time
  int Gen_QQ_momId[Max_QQ_size];    // pdgId of mother particle of 2 muons
  
  int Gen_mu_size; // number of generated muons
  int Gen_mu_charge[Max_mu_size]; // muon charge
  int Gen_mu_type[Max_mu_size]; // muon type: prompt, non-prompt, unmatched

  int Reco_QQ_size;       // Number of reconstructed Onia 
  int Reco_QQ_type[Max_QQ_size];   // Onia category: GG, GT, TT
  int Reco_QQ_sign[Max_QQ_size];   /* Mu Mu combinations sign:
                             0 = +/- (signal)
                             1 = +/+
                             2 = -/- 
                          */
  ULong64_t Reco_QQ_trig[Max_QQ_size];      // Vector of trigger bits matched to the Onia
  ULong64_t Reco_QQ_mupl_trig[Max_QQ_size];      // Vector of trigger bits matched to the Onia
  ULong64_t Reco_QQ_mumi_trig[Max_QQ_size];      // Vector of trigger bits matched to the Onia
  bool Reco_QQ_isCowboy[Max_mu_size]; // Cowboy/Sailor Flag 
  float Reco_QQ_VtxProb[Max_QQ_size]; // chi2 probability of vertex fitting 
  float Reco_QQ_ctau[Max_QQ_size];    // ctau: flight time
  float Reco_QQ_ctauErr[Max_QQ_size]; // error on ctau
  float Reco_QQ_ctauTrue[Max_QQ_size];// true ctau
  float Reco_QQ_ctau3D[Max_QQ_size];    // ctau: flight time in 3D
  float Reco_QQ_ctauErr3D[Max_QQ_size]; // error on ctau in 3D
  float Reco_QQ_ctauTrue3D[Max_QQ_size];// true ctau in 3D
  float Reco_QQ_dca[Max_QQ_size];
  float Reco_QQ_MassErr[Max_QQ_size];

  int  Reco_QQ_Ntrk[Max_QQ_size];
  int  Reco_QQ_NtrkPt02[Max_QQ_size];
  int  Reco_QQ_NtrkPt03[Max_QQ_size];
  int  Reco_QQ_NtrkPt04[Max_QQ_size];

  int  Reco_QQ_NtrkDeltaR03[Max_QQ_size];
  int  Reco_QQ_NtrkDeltaR04[Max_QQ_size];
  int  Reco_QQ_NtrkDeltaR05[Max_QQ_size];

  bool Reco_QQ_mupl_isGoodMuon[Max_QQ_size];      // Vector of isGoodMuon(TMOneStationTight) for plus muon
  bool Reco_QQ_mumi_isGoodMuon[Max_QQ_size];      // Vector of isGoodMuon(TMOneStationTight) for minus muon
  bool Reco_QQ_mupl_highPurity[Max_QQ_size];      // Vector of high purity flag for plus muon
  bool Reco_QQ_mumi_highPurity[Max_QQ_size];      // Vector of high purity flag for minus muon
  bool Reco_QQ_mupl_TrkMuArb[Max_QQ_size];      // Vector of TrackerMuonArbitrated for plus muon
  bool Reco_QQ_mumi_TrkMuArb[Max_QQ_size];      // Vector of TrackerMuonArbitrated for minus muon
  bool Reco_QQ_mupl_TMOneStaTight[Max_QQ_size]; // Vector of TMOneStationTight for plus muon
  bool Reco_QQ_mumi_TMOneStaTight[Max_QQ_size]; // Vector of TMOneStationTight for minus muon

  int Reco_QQ_mupl_SelectionType[Max_QQ_size];  
  int Reco_QQ_mumi_SelectionType[Max_QQ_size];  
  int Reco_QQ_mupl_nPixValHits[Max_QQ_size];  // Number of valid pixel hits in plus sta muons
  int Reco_QQ_mumi_nPixValHits[Max_QQ_size];  // Number of valid pixel hits in minus sta muons
  int Reco_QQ_mupl_nMuValHits[Max_QQ_size];  // Number of valid muon hits in plus sta muons
  int Reco_QQ_mumi_nMuValHits[Max_QQ_size];  // Number of valid muon hits in minus sta muons
  int Reco_QQ_mupl_nTrkHits[Max_QQ_size];  // track hits plus global muons
  int Reco_QQ_mumi_nTrkHits[Max_QQ_size];  // track hits minus global muons
  int Reco_QQ_mupl_nPixWMea[Max_QQ_size];  // pixel layers with measurement for plus inner track muons
  int Reco_QQ_mumi_nPixWMea[Max_QQ_size];  // pixel layers with measurement for minus inner track muons
  int Reco_QQ_mupl_nTrkWMea[Max_QQ_size];  // track layers with measurement for plus inner track muons
  int Reco_QQ_mumi_nTrkWMea[Max_QQ_size];  // track layers with measurement for minus inner track muons
  int Reco_QQ_mupl_StationsMatched[Max_QQ_size];  // number of stations matched for plus inner track muons
  int Reco_QQ_mumi_StationsMatched[Max_QQ_size];  // number of stations matched for minus inner track muons
  float Reco_QQ_mupl_normChi2_inner[Max_QQ_size];  // chi2/ndof for plus inner track muons
  float Reco_QQ_mumi_normChi2_inner[Max_QQ_size];  // chi2/ndof for minus inner track muons
  float Reco_QQ_mupl_normChi2_global[Max_QQ_size];  // chi2/ndof for plus global muons
  float Reco_QQ_mumi_normChi2_global[Max_QQ_size];  // chi2/ndof for minus global muons
  float Reco_QQ_mupl_dxy[Max_QQ_size];  // dxy for plus inner track muons
  float Reco_QQ_mumi_dxy[Max_QQ_size];  // dxy for minus inner track muons
  float Reco_QQ_mupl_dxyErr[Max_QQ_size];  // dxy error for plus inner track muons
  float Reco_QQ_mumi_dxyErr[Max_QQ_size];  // dxy error for minus inner track muons
  float Reco_QQ_mupl_dz[Max_QQ_size];  // dz for plus inner track muons
  float Reco_QQ_mumi_dz[Max_QQ_size];  // dz for minus inner track muons
  float Reco_QQ_mupl_dzErr[Max_QQ_size];  // dz error for plus inner track muons
  float Reco_QQ_mumi_dzErr[Max_QQ_size];  // dz error for minus inner track muons
  float Reco_QQ_mupl_pt_inner[Max_QQ_size];  // pT for plus inner track muons
  float Reco_QQ_mumi_pt_inner[Max_QQ_size];  // pT for minus inner track muons
  float Reco_QQ_mupl_pt_global[Max_QQ_size];  // pT for plus global muons
  float Reco_QQ_mumi_pt_global[Max_QQ_size];  // pT for minus global muons
  float Reco_QQ_mupl_ptErr_inner[Max_QQ_size];  // pT error for plus inner track muons
  float Reco_QQ_mumi_ptErr_inner[Max_QQ_size];  // pT error for minus inner track muons
  float Reco_QQ_mupl_ptErr_global[Max_QQ_size];  // pT error for plus global muons
  float Reco_QQ_mumi_ptErr_global[Max_QQ_size];  // pT error for minus global muons

  int Reco_mu_size;           // Number of reconstructed muons
  int Reco_mu_SelectionType[Max_mu_size];           
  ULong64_t Reco_mu_trig[Max_mu_size];      // Vector of trigger bits matched to the muon
  int Reco_mu_charge[Max_mu_size];  // Vector of charge of muons
  int Reco_mu_type[Max_mu_size];  // Vector of type of muon (global=0, tracker=1, calo=2)  

  bool Reco_mu_isGoodMuon[Max_mu_size];    // Vector of isGoodMuon(TMOneStationTight)
  bool Reco_mu_highPurity[Max_mu_size];    // Vector of high purity flag  
  bool Reco_mu_TrkMuArb[Max_mu_size];      // Vector of TrackerMuonArbitrated
  bool Reco_mu_TMOneStaTight[Max_mu_size]; // Vector of TMOneStationTight

  int Reco_mu_nPixValHits[Max_mu_size];  // Number of valid pixel hits in sta muons
  int Reco_mu_nMuValHits[Max_mu_size];  // Number of valid muon hits in sta muons
  int Reco_mu_nTrkHits[Max_mu_size];  // track hits global muons
  int Reco_mu_nPixWMea[Max_mu_size];  // pixel layers with measurement for inner track muons
  int Reco_mu_nTrkWMea[Max_mu_size];  // track layers with measurement for inner track muons
  int Reco_mu_StationsMatched[Max_mu_size];  // number of stations matched for inner track muons
  float Reco_mu_normChi2_inner[Max_mu_size];  // chi2/ndof for inner track muons
  float Reco_mu_normChi2_global[Max_mu_size];  // chi2/ndof for global muons
  float Reco_mu_dxy[Max_mu_size];  // dxy for inner track muons
  float Reco_mu_dxyErr[Max_mu_size];  // dxy error for inner track muons
  float Reco_mu_dz[Max_mu_size];  // dz for inner track muons
  float Reco_mu_dzErr[Max_mu_size];  // dz error for inner track muons
  float Reco_mu_pt_inner[Max_mu_size];  // pT for inner track muons
  float Reco_mu_pt_global[Max_mu_size];  // pT for global muons
  float Reco_mu_ptErr_inner[Max_mu_size];  // pT error for inner track muons
  float Reco_mu_ptErr_global[Max_mu_size];  // pT error for global muons
  
  int muType; // type of muon (GlbTrk=0, Trk=1, Glb=2, none=-1) 

  int Reco_trk_size;           // Number of reconstructed tracks
  int Reco_trk_charge[Max_trk_size];  // Vector of charge of tracks
  float Reco_trk_dxyError[Max_trk_size];
  float Reco_trk_dzError[Max_trk_size];

  // histos
  TH1F* hGoodMuonsNoTrig;
  TH1F* hGoodMuons;
  TH1F* hL1DoubleMu0;

  MyCommonHistoManager* myRecoMuonHistos;
  MyCommonHistoManager* myRecoGlbMuonHistos;
  MyCommonHistoManager* myRecoTrkMuonHistos;

  MyCommonHistoManager* myRecoJpsiHistos;
  MyCommonHistoManager* myRecoJpsiGlbGlbHistos;
  MyCommonHistoManager* myRecoJpsiGlbTrkHistos;
  MyCommonHistoManager* myRecoJpsiTrkTrkHistos;

  // event counters
  TH1F* hStats;

  // centrality
  TH1F *hCent;

  // number of primary vertices
  TH1F* hPileUp;

  // z vertex distribution
  TH1F* hZVtx;

  // centrality
  int centBin;
  int theCentralityBin;

  int Npix, NpixelTracks, Ntracks,  NtracksPtCut, NtracksEtaCut, NtracksEtaPtCut;
  float SumET_HF, SumET_HFplus, SumET_HFminus, SumET_HFplusEta4, SumET_HFminusEta4,SumET_HFhit, SumET_HFhitPlus, SumET_HFhitMinus, SumET_EB, SumET_ET, SumET_EE, SumET_EEplus, SumET_EEminus, SumET_ZDC, SumET_ZDCplus, SumET_ZDCminus;

  // Event Plane variables
  int nEP;   // number of event planes
  float *hiEvtPlane;
  float rpAng[50];
  float rpCos[50];
  float rpSin[50];

  // handles
  edm::Handle<pat::CompositeCandidateCollection> collJpsi;
  edm::Handle<pat::MuonCollection> collMuon;
  edm::Handle<pat::MuonCollection> collMuonNoTrig;
  edm::Handle<reco::TrackCollection> collTracks;

  edm::Handle<reco::GenParticleCollection> collGenParticles;

  edm::Handle<edm::TriggerResults> collTriggerResults;

  // data members
  edm::EDGetTokenT<pat::MuonCollection>               _patMuonToken;
  edm::EDGetTokenT<pat::MuonCollection>               _patMuonNoTrigToken;
  edm::EDGetTokenT<pat::CompositeCandidateCollection> _patJpsiToken;
  edm::EDGetTokenT<reco::TrackCollection>             _recoTracksToken;
  edm::EDGetTokenT<reco::GenParticleCollection>       _genParticleToken;
  edm::EDGetTokenT<reco::VertexCollection>            _thePVsToken;
  edm::EDGetTokenT<edm::TriggerResults>               _tagTriggerResultsToken;
  edm::EDGetTokenT<reco::Centrality>                  _centralityTagToken;
  edm::EDGetTokenT<int>                               _centralityBinTagToken;
  edm::EDGetTokenT<reco::EvtPlaneCollection>          _evtPlaneTagToken;
  std::string         _histfilename;
  std::string         _datasetname;
  std::string         _muonSel;
 
  std::vector<double> _centralityranges;
  std::vector<double> _ptbinranges;
  std::vector<double> _etabinranges;
  std::vector<string> _dblTriggerPathNames;
  std::vector<string> _dblTriggerFilterNames;
  std::vector<string> _sglTriggerPathNames;
  std::vector<string> _sglTriggerFilterNames;

  bool           _onlythebest;
  bool           _applycuts;
  bool           _selTightGlobalMuon;
  bool           _storeefficiency;
  bool           _muonLessPrimaryVertex;
  bool           _useBS;
  bool           _useRapidity;
  bool           _removeSignal;
  bool           _removeMuons;
  bool           _storeSs;
  bool           _combineCategories;
  bool           _fillRooDataSet;
  bool           _fillTree;
  bool           _fillHistos;
  bool           _theMinimumFlag;
  bool           _fillSingleMuons;
  bool           _fillRecoTracks;
  bool           _isHI;
  bool           _isPA;
  bool           _isMC;
  bool           _isPromptMC;
  bool           _useEvtPlane;
  bool           _useGeTracks;

  int _oniaPDG;

  std::vector<unsigned int>                     _thePassedCats[3];
  std::vector<const pat::CompositeCandidate*>   _thePassedCands[3];

  std::vector<uint32_t>   _thePassedPFPhotons;
  std::vector<uint32_t>   _thePassedPFPhotonsForPi0Rejection;

  // number of events
  unsigned int nEvents;
  unsigned int passedCandidates;

  unsigned int runNb;
  unsigned int eventNb;
  unsigned int lumiSection;

  // limits 
  float JpsiMassMin;
  float JpsiMassMax;
  float JpsiPtMin;           // SET BY 
  float JpsiPtMax;           // DEFINITION
  float JpsiRapMin;          // OF BIN
  float JpsiRapMax;          // LIMITS 

  math::XYZPoint RefVtx;
  float RefVtx_xError;
  float RefVtx_yError;
  float RefVtx_zError;
  float zVtx;
  float nPV;

 // Triger stuff
  // PUT HERE THE *LAST FILTERS* OF THE BITS YOU LIKE
  static const unsigned int sNTRIGGERS = 65;
  unsigned int NTRIGGERS;
  unsigned int NTRIGGERS_DBL;
  unsigned int nTrig;

  // MC 8E29
  bool isTriggerMatched[sNTRIGGERS];
  std::string HLTLastFilters[sNTRIGGERS];
  bool alreadyFilled[sNTRIGGERS];
  ULong64_t HLTriggers;
  int trigPrescale[sNTRIGGERS];

  std::map<std::string, int> mapTriggerNameToIntFired_;
  std::map<std::string, int> mapTriggerNameToPrescaleFac_;

  HLTPrescaleProvider hltPrescaleProvider;
  bool hltPrescaleInit;

  const edm::ParameterSet _iConfig;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
HiOniaAnalyzer::HiOniaAnalyzer(const edm::ParameterSet& iConfig):
  _patMuonToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuon"))),
  _patMuonNoTrigToken(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("srcMuonNoTrig"))),
  _patJpsiToken(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter<edm::InputTag>("src"))),
  _recoTracksToken(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
  _genParticleToken(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
  _thePVsToken(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
  _tagTriggerResultsToken(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("triggerResultsLabel"))),
  _centralityTagToken(consumes<reco::Centrality>(iConfig.getParameter<edm::InputTag> ("CentralitySrc"))),
  _centralityBinTagToken(consumes<int>(iConfig.getParameter<edm::InputTag> ("CentralityBinSrc"))),
  _evtPlaneTagToken(consumes<reco::EvtPlaneCollection>(iConfig.getParameter<edm::InputTag> ("EvtPlane"))),
  _histfilename(iConfig.getParameter<std::string>("histFileName")),             
  _datasetname(iConfig.getParameter<std::string>("dataSetName")),         
  _muonSel(iConfig.getParameter<std::string>("muonSel")),
  _centralityranges(iConfig.getParameter< std::vector<double> >("centralityRanges")),           
  _ptbinranges(iConfig.getParameter< std::vector<double> >("pTBinRanges")),     
  _etabinranges(iConfig.getParameter< std::vector<double> >("etaBinRanges")),   
  _dblTriggerPathNames(iConfig.getParameter< std::vector<string> >("dblTriggerPathNames")),
  _dblTriggerFilterNames(iConfig.getParameter< std::vector<string> >("dblTriggerFilterNames")),
  _sglTriggerPathNames(iConfig.getParameter< std::vector<string> >("sglTriggerPathNames")),
  _sglTriggerFilterNames(iConfig.getParameter< std::vector<string> >("sglTriggerFilterNames")),
  _onlythebest(iConfig.getParameter<bool>("onlyTheBest")),              
  _applycuts(iConfig.getParameter<bool>("applyCuts")),            
  _selTightGlobalMuon(iConfig.getParameter<bool>("selTightGlobalMuon")),                      
  _storeefficiency(iConfig.getParameter<bool>("storeEfficiency")),      
  _muonLessPrimaryVertex(iConfig.getParameter<bool>("muonLessPV")),
  _useBS(iConfig.getParameter<bool>("useBeamSpot")),
  _useRapidity(iConfig.getParameter<bool>("useRapidity")),
  _removeSignal(iConfig.getUntrackedParameter<bool>("removeSignalEvents",false)),
  _removeMuons(iConfig.getUntrackedParameter<bool>("removeTrueMuons",false)),
  _storeSs(iConfig.getUntrackedParameter<bool>("storeSameSign",false)),
  _combineCategories(iConfig.getParameter<bool>("combineCategories")),
  _fillRooDataSet(iConfig.getParameter<bool>("fillRooDataSet")),  
  _fillTree(iConfig.getParameter<bool>("fillTree")),  
  _fillHistos(iConfig.getParameter<bool>("fillHistos")),
  _theMinimumFlag(iConfig.getParameter<bool>("minimumFlag")),  
  _fillSingleMuons(iConfig.getParameter<bool>("fillSingleMuons")),
  _fillRecoTracks(iConfig.getParameter<bool>("fillRecoTracks")),
  _isHI(iConfig.getUntrackedParameter<bool>("isHI",false) ),
  _isPA(iConfig.getUntrackedParameter<bool>("isPA",true) ),
  _isMC(iConfig.getUntrackedParameter<bool>("isMC",false) ),
  _isPromptMC(iConfig.getUntrackedParameter<bool>("isPromptMC",true) ),
  _useEvtPlane(iConfig.getUntrackedParameter<bool>("useEvtPlane",false) ),
  _useGeTracks(iConfig.getUntrackedParameter<bool>("useGeTracks",false) ),
  _oniaPDG(iConfig.getParameter<int>("oniaPDG")),
  hltPrescaleProvider(iConfig, consumesCollector(), *this),
  _iConfig(iConfig)
{
   //now do what ever initialization is needed
  nEvents = 0;
  passedCandidates = 0;

  theRegions.push_back("All");
  theRegions.push_back("Barrel");
  theRegions.push_back("EndCap");

  std::stringstream centLabel;
  for (unsigned int iCent=0; iCent<_centralityranges.size(); ++iCent) {
    if (iCent==0)
      centLabel << "00" << _centralityranges.at(iCent);
    else
      centLabel << _centralityranges.at(iCent-1) << _centralityranges.at(iCent);

    theCentralities.push_back(centLabel.str());
    centLabel.str("");
  }
  theCentralities.push_back("MinBias");

  theSign.push_back("pm");
  if (_storeSs) {
    theSign.push_back("pp");
    theSign.push_back("mm");
  }

  NTRIGGERS_DBL = _dblTriggerPathNames.size();
  NTRIGGERS = NTRIGGERS_DBL + _sglTriggerPathNames.size() + 1; // + 1 for "NoTrigger"
  std::cout << "NTRIGGERS_DBL = " << NTRIGGERS_DBL << "\t NTRIGGERS_SGL = " << _sglTriggerPathNames.size() << "\t NTRIGGERS = " << NTRIGGERS << std::endl;
  nTrig = NTRIGGERS - 1;

  isTriggerMatched[0]=true; // first entry 'hardcoded' true to accept "all" events
  HLTLastFilters[0] = "";
  theTriggerNames.push_back("NoTrigger");

  for (unsigned int iTr = 1; iTr<NTRIGGERS; ++iTr) {
    isTriggerMatched[iTr] = false;

    if (iTr<=NTRIGGERS_DBL) {
      HLTLastFilters[iTr] = _dblTriggerFilterNames.at(iTr-1);
      theTriggerNames.push_back(_dblTriggerPathNames.at(iTr-1));
    }
    else {
      HLTLastFilters[iTr] = _sglTriggerFilterNames.at(iTr-NTRIGGERS_DBL-1);
      theTriggerNames.push_back(_sglTriggerPathNames.at(iTr-NTRIGGERS_DBL-1));
    }
    std::cout<<" Trigger "<<iTr<<"\t"<<HLTLastFilters[iTr]<<std::endl;
  }

  etaMax = 2.4;

  JpsiMassMin = 2.6;
  JpsiMassMax = 3.5;

  JpsiPtMin = _ptbinranges[0];
  std::cout << "Pt min = " << JpsiPtMin << std::endl;
  JpsiPtMax = _ptbinranges[_ptbinranges.size()-1];
  std::cout << "Pt max = " << JpsiPtMax << std::endl;

     
  JpsiRapMin = _etabinranges[0];
  std::cout << "Rap min = " << JpsiRapMin << std::endl;
  JpsiRapMax = _etabinranges[_etabinranges.size()-1];
  std::cout << "Rap max = " << JpsiRapMax << std::endl;
  

  for(std::vector<std::string>::iterator it = theTriggerNames.begin(); it != theTriggerNames.end(); ++it){
      mapTriggerNameToIntFired_[*it] = -9999;
      mapTriggerNameToPrescaleFac_[*it] = -1;
  }
}


HiOniaAnalyzer::~HiOniaAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
HiOniaAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //   using namespace edm;
  InitEvent();

  nEvents++;
  hStats->Fill(BIN_nEvents);
   
  runNb = iEvent.id().run();
  eventNb = iEvent.id().event();
  lumiSection = iEvent.luminosityBlock();
  
  edm::Handle<reco::VertexCollection> privtxs;
  iEvent.getByToken(_thePVsToken, privtxs); 
  reco::VertexCollection::const_iterator privtx;

  nPV = privtxs->size();
  
  if ( privtxs->begin() != privtxs->end() ) {
    privtx=privtxs->begin();
    RefVtx = privtx->position();
    RefVtx_xError = privtx->xError();
    RefVtx_yError = privtx->yError();
    RefVtx_zError = privtx->zError();
  } else {
    RefVtx.SetXYZ(0.,0.,0.);
    RefVtx_xError = 0.0;
    RefVtx_yError = 0.0;
    RefVtx_zError = 0.0;
  }

  zVtx = RefVtx.Z();

  hZVtx->Fill(zVtx);
  hPileUp->Fill(nPV);

  this->hltReport(iEvent, iSetup);

  for (unsigned int iTr = 1 ; iTr < theTriggerNames.size() ; iTr++) {
    if (mapTriggerNameToIntFired_[theTriggerNames.at(iTr)] == 3) {
      HLTriggers += pow(2,iTr-1);
      hStats->Fill(iTr); // event info
    }
    trigPrescale[iTr-1] = mapTriggerNameToPrescaleFac_[theTriggerNames.at(iTr)];
  }

  if (_isHI || _isPA) {

    edm::Handle<reco::Centrality> centrality;
    iEvent.getByToken(_centralityTagToken, centrality); 

    edm::Handle<int> cbin_;
    iEvent.getByToken(_centralityBinTagToken, cbin_);
    centBin = *cbin_;
    hCent->Fill(centBin);
    
    for (unsigned int iCent=0; iCent<_centralityranges.size(); ++iCent) {
      if ( (_isHI && centBin<_centralityranges.at(iCent)/0.5) ||
           (_isPA && centBin<_centralityranges.at(iCent)) ) {
        theCentralityBin=iCent;
        break;
      }
    }

    Npix            = centrality->multiplicityPixel();
    NpixelTracks    = centrality->NpixelTracks();
    Ntracks         = centrality->Ntracks();
    NtracksPtCut    = centrality->NtracksPtCut();
    NtracksEtaCut   = centrality->NtracksEtaCut();
    NtracksEtaPtCut = centrality->NtracksEtaPtCut();

    SumET_HF          = centrality->EtHFtowerSum();
    SumET_HFplus      = centrality->EtHFtowerSumPlus();
    SumET_HFminus     = centrality->EtHFtowerSumMinus();
    SumET_HFplusEta4  = centrality->EtHFtruncatedPlus();
    SumET_HFminusEta4 = centrality->EtHFtruncatedMinus();

    SumET_HFhit       = centrality->EtHFhitSum(); 
    SumET_HFhitPlus   = centrality->EtHFhitSumPlus();
    SumET_HFhitMinus  = centrality->EtHFhitSumMinus();

    SumET_ZDC         = centrality->zdcSum();
    SumET_ZDCplus     = centrality->zdcSumPlus();
    SumET_ZDCminus    = centrality->zdcSumMinus();

    SumET_EEplus      = centrality->EtEESumPlus();
    SumET_EEminus     = centrality->EtEESumMinus();
    SumET_EE          = centrality->EtEESum();
    SumET_EB          = centrality->EtEBSum();
    SumET_ET          = centrality->EtMidRapiditySum();
  }
  else {
    centBin = 0;
    theCentralityBin=0;

    Npix = 0;
    NpixelTracks = 0;
    Ntracks = 0;
    NtracksPtCut    = 0;
    NtracksEtaCut   = 0;
    NtracksEtaPtCut = 0;

    SumET_HF = 0;
    SumET_HFplus = 0;
    SumET_HFminus = 0;
    SumET_HFplusEta4 = 0;
    SumET_HFminusEta4 = 0;

    SumET_HFhit       = 0;
    SumET_HFhitPlus   = 0; 
    SumET_HFhitMinus  = 0;

    SumET_ZDC = 0;
    SumET_ZDCplus = 0;
    SumET_ZDCminus = 0;
    SumET_EEplus = 0;
    SumET_EEminus = 0;
    SumET_EE = 0;
    SumET_EB = 0;
    SumET_ET = 0;
  }

  if (_isHI && _useEvtPlane) {
    nEP = 0; 
    edm::Handle<reco::EvtPlaneCollection> flatEvtPlanes;
    iEvent.getByToken(_evtPlaneTagToken,flatEvtPlanes);
    if(flatEvtPlanes.isValid()) {
      for (reco::EvtPlaneCollection::const_iterator rp = flatEvtPlanes->begin(); rp!=flatEvtPlanes->end(); rp++) {
        rpAng[nEP] = rp->angle(2);   // Using Event Plane Level 2 -> Includes recentering and flattening. 
        rpSin[nEP] = rp->sumSin(2);  // Using Event Plane Level 2 -> Includes recentering and flattening. 
        rpCos[nEP] = rp->sumCos(2);  // Using Event Plane Level 2 -> Includes recentering and flattening. 
        nEP++;
      }
    } else if (!_isMC) {
      std::cout << "Warning! Can't get flattened hiEvtPlane product!" << std::endl;
    }
  }

  iEvent.getByToken(_patJpsiToken,collJpsi); 
  iEvent.getByToken(_patMuonToken,collMuon);
  iEvent.getByToken(_patMuonNoTrigToken,collMuonNoTrig);

  if (_isMC) {
    iEvent.getByToken(_genParticleToken,collGenParticles);
    this->fillGenInfo();
  }
  
  // APPLY CUTS
  int lastSign = 0;
  this->makeCuts(0);
  if (_storeSs) {
    this->makeCuts(1);
    this->makeCuts(2);
    lastSign = 2;
  }

  if (_fillSingleMuons)
    this->fillRecoMuons(theCentralityBin);

  if (_useGeTracks){
    iEvent.getByToken(_recoTracksToken,collTracks);
    if (_fillRecoTracks)
      this->fillRecoTracks();
  }

  this->fillRecoHistos(lastSign);

  if (_fillTree)
    myTree->Fill();

  return;
}

void
HiOniaAnalyzer::fillRecoHistos(int lastSign) {

  // BEST J/PSI? 
  if (_onlythebest) {  // yes, fill simply the best (possibly same-sign)

    for (int iSign = 0; iSign <= lastSign; ++iSign) {
      pair< unsigned int, const pat::CompositeCandidate* > theBest = theBestQQ(iSign);
      if (theBest.first < 10) this->fillHistosAndDS(theBest.first, theBest.second);
    }

  } else {   // no, fill all candidates passing cuts (possibly same-sign)
   
    for (int iSign = 0; iSign <= lastSign; ++iSign) {
      for( unsigned int count = 0; count < _thePassedCands[iSign].size(); count++) { 
        const pat::CompositeCandidate* aJpsiCand = _thePassedCands[iSign].at(count); 

        this->checkTriggers(aJpsiCand);
        if (_fillTree)
          this->fillTreeJpsi(iSign, count);

        for (unsigned int iTr=0; iTr<NTRIGGERS; ++iTr) {
          if (isTriggerMatched[iTr]) {
            this->fillRecoJpsi(iSign,count,theTriggerNames.at(iTr), theCentralities.at(theCentralityBin));
          }
        }
      }
    }
  }

  return;
}

void
HiOniaAnalyzer::fillTreeMuon(const pat::Muon* muon, int iType, ULong64_t trigBits) {
  if (Reco_mu_size >= Max_mu_size) {
    std::cout << "Too many muons: " << Reco_mu_size << std::endl;
    std::cout << "Maximum allowed: " << Max_mu_size << std::endl;
    return;
  }

  Reco_mu_charge[Reco_mu_size] = muon->charge();
  Reco_mu_type[Reco_mu_size] = iType;
  
  TLorentzVector vMuon = lorentzMomentum(muon->p4());
  new((*Reco_mu_4mom)[Reco_mu_size])TLorentzVector(vMuon);

  Reco_mu_trig[Reco_mu_size] = trigBits;

  reco::TrackRef iTrack = muon->innerTrack();
  
  if (!_theMinimumFlag) {
    Reco_mu_SelectionType[Reco_mu_size] = muonIDmask(muon);
    Reco_mu_highPurity[Reco_mu_size] = iTrack->quality(reco::TrackBase::highPurity);
    Reco_mu_isGoodMuon[Reco_mu_size] = muon::isGoodMuon(*muon, muon::TMOneStationTight);
    Reco_mu_TrkMuArb[Reco_mu_size] = muon->muonID("TrackerMuonArbitrated");
    Reco_mu_TMOneStaTight[Reco_mu_size] = muon->muonID("TMOneStationTight");
    Reco_mu_nTrkHits[Reco_mu_size] = iTrack->found();
    Reco_mu_normChi2_inner[Reco_mu_size] = iTrack->normalizedChi2();
    Reco_mu_nPixValHits[Reco_mu_size] = iTrack->hitPattern().numberOfValidPixelHits();
    Reco_mu_nPixWMea[Reco_mu_size] = iTrack->hitPattern().pixelLayersWithMeasurement();
    Reco_mu_nTrkWMea[Reco_mu_size] = iTrack->hitPattern().trackerLayersWithMeasurement();
    Reco_mu_StationsMatched[Reco_mu_size] = muon->numberOfMatchedStations();
    Reco_mu_dxy[Reco_mu_size] = iTrack->dxy(RefVtx);
    Reco_mu_dxyErr[Reco_mu_size] = iTrack->dxyError();
    Reco_mu_dz[Reco_mu_size] = iTrack->dz(RefVtx);
    Reco_mu_dzErr[Reco_mu_size] = iTrack->dzError();
    Reco_mu_pt_inner[Reco_mu_size] = iTrack->pt();
    Reco_mu_ptErr_inner[Reco_mu_size] = iTrack->ptError();
    
    if (muon->isGlobalMuon()) {
      reco::TrackRef gTrack = muon->globalTrack();
      Reco_mu_nMuValHits[Reco_mu_size] = gTrack->hitPattern().numberOfValidMuonHits();
      Reco_mu_normChi2_global[Reco_mu_size] = gTrack->normalizedChi2();
      Reco_mu_pt_global[Reco_mu_size] = gTrack->pt();
      Reco_mu_ptErr_global[Reco_mu_size] = gTrack->ptError();
    }
    else {
      Reco_mu_nMuValHits[Reco_mu_size] = -1;
      Reco_mu_normChi2_global[Reco_mu_size] = 999;
      Reco_mu_pt_global[Reco_mu_size] = -1;
      Reco_mu_ptErr_global[Reco_mu_size] = -1;
    }
  }

  Reco_mu_size++;
  return;
}

void
HiOniaAnalyzer::fillTreeJpsi(int iSign, int count) {
  if (Reco_QQ_size >= Max_QQ_size) {
    std::cout << "Too many dimuons: " << Reco_QQ_size << std::endl;
    std::cout << "Maximum allowed: " << Max_QQ_size << std::endl;
    return;
  }

  const pat::CompositeCandidate* aJpsiCand = _thePassedCands[iSign].at(count);
  const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
  const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));

  ULong64_t trigBits=0, trigBits_mu1=0, trigBits_mu2=0;
  for (unsigned int iTr=1; iTr<NTRIGGERS; ++iTr) {
    if (isTriggerMatched[iTr]) {trigBits += pow(2,iTr-1);}

    const pat::TriggerObjectStandAloneCollection mu1HLTMatchesFilter = muon1->triggerObjectMatchesByFilter( HLTLastFilters[iTr] );
    const pat::TriggerObjectStandAloneCollection mu2HLTMatchesFilter = muon2->triggerObjectMatchesByFilter( HLTLastFilters[iTr] );
    if (mu1HLTMatchesFilter.size() > 0) { trigBits_mu1 += pow(2,iTr-1); }
    if (mu2HLTMatchesFilter.size() > 0) { trigBits_mu2 += pow(2,iTr-1); }
  }

  Reco_QQ_sign[Reco_QQ_size] = iSign;
  Reco_QQ_type[Reco_QQ_size] = _thePassedCats[iSign].at(count);

  Reco_QQ_trig[Reco_QQ_size] = trigBits;

  if (!(_isHI || _isPA) && _muonLessPrimaryVertex && aJpsiCand->hasUserData("muonlessPV")) {
    RefVtx = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).position();
    RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).xError();
    RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).yError();
    RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("muonlessPV")).zError();
  }
  else if (!_muonLessPrimaryVertex && aJpsiCand->hasUserData("PVwithmuons")) {
    RefVtx = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).position();
    RefVtx_xError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).xError();
    RefVtx_yError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).yError();
    RefVtx_zError = (*aJpsiCand->userData<reco::Vertex>("PVwithmuons")).zError();
  }
  else {
    cout << "HiOniaAnalyzer::fillTreeJpsi: no PVfor muon pair stored" << endl;
    return;
  }

  new((*Reco_QQ_vtx)[Reco_QQ_size])TVector3(RefVtx.X(),RefVtx.Y(),RefVtx.Z());

  TLorentzVector vMuon1 = lorentzMomentum(muon1->p4());
  TLorentzVector vMuon2 = lorentzMomentum(muon2->p4());

  reco::TrackRef iTrack_mupl;
  reco::TrackRef gTrack_mupl;

  reco::TrackRef iTrack_mumi;
  reco::TrackRef gTrack_mumi;

  if (muon1->charge() > muon2->charge()) {
    new((*Reco_QQ_mupl_4mom)[Reco_QQ_size])TLorentzVector(vMuon1);
    new((*Reco_QQ_mumi_4mom)[Reco_QQ_size])TLorentzVector(vMuon2);

    Reco_QQ_mupl_trig[Reco_QQ_size] = trigBits_mu1;
    Reco_QQ_mumi_trig[Reco_QQ_size] = trigBits_mu2;

    Reco_QQ_mupl_SelectionType[Reco_QQ_size] = muonIDmask(muon1);
    Reco_QQ_mumi_SelectionType[Reco_QQ_size] = muonIDmask(muon2);

    if (TVector2::Phi_mpi_pi(vMuon1.Phi() - vMuon2.Phi()) > 0) Reco_QQ_isCowboy[Reco_QQ_size] = true;
    else Reco_QQ_isCowboy[Reco_QQ_size] = false;

    Reco_QQ_mupl_StationsMatched[Reco_QQ_size] = muon1->numberOfMatchedStations();
    Reco_QQ_mumi_StationsMatched[Reco_QQ_size] = muon2->numberOfMatchedStations();

    Reco_QQ_mupl_isGoodMuon[Reco_QQ_size] = muon::isGoodMuon(*muon1, muon::TMOneStationTight);
    Reco_QQ_mumi_isGoodMuon[Reco_QQ_size] = muon::isGoodMuon(*muon2, muon::TMOneStationTight);

    Reco_QQ_mupl_TrkMuArb[Reco_QQ_size] = muon1->muonID("TrackerMuonArbitrated");
    Reco_QQ_mupl_TMOneStaTight[Reco_QQ_size] = muon1->muonID("TMOneStationTight");

    Reco_QQ_mumi_TrkMuArb[Reco_QQ_size] = muon2->muonID("TrackerMuonArbitrated");
    Reco_QQ_mumi_TMOneStaTight[Reco_QQ_size] = muon2->muonID("TMOneStationTight");

    iTrack_mupl = muon1->innerTrack();
    iTrack_mumi = muon2->innerTrack();

    if (muon1->isGlobalMuon())
      gTrack_mupl = muon1->globalTrack();

    if (muon2->isGlobalMuon())
      gTrack_mumi = muon2->globalTrack();
  }
  else {
    new((*Reco_QQ_mupl_4mom)[Reco_QQ_size])TLorentzVector(vMuon2);
    new((*Reco_QQ_mumi_4mom)[Reco_QQ_size])TLorentzVector(vMuon1);

    Reco_QQ_mupl_trig[Reco_QQ_size] = trigBits_mu2;
    Reco_QQ_mumi_trig[Reco_QQ_size] = trigBits_mu1;

    Reco_QQ_mupl_SelectionType[Reco_QQ_size] = muonIDmask(muon2);
    Reco_QQ_mumi_SelectionType[Reco_QQ_size] = muonIDmask(muon1);

    if (TVector2::Phi_mpi_pi(vMuon2.Phi() - vMuon1.Phi()) > 0) Reco_QQ_isCowboy[Reco_QQ_size] = true;
    else Reco_QQ_isCowboy[Reco_QQ_size] = false;

    Reco_QQ_mupl_StationsMatched[Reco_QQ_size] = muon2->numberOfMatchedStations();
    Reco_QQ_mumi_StationsMatched[Reco_QQ_size] = muon1->numberOfMatchedStations();

    Reco_QQ_mupl_isGoodMuon[Reco_QQ_size] = muon::isGoodMuon(*muon2, muon::TMOneStationTight);
    Reco_QQ_mumi_isGoodMuon[Reco_QQ_size] = muon::isGoodMuon(*muon1, muon::TMOneStationTight);

    Reco_QQ_mupl_TrkMuArb[Reco_QQ_size] = muon2->muonID("TrackerMuonArbitrated");
    Reco_QQ_mupl_TMOneStaTight[Reco_QQ_size] = muon2->muonID("TMOneStationTight");

    Reco_QQ_mumi_TrkMuArb[Reco_QQ_size] = muon1->muonID("TrackerMuonArbitrated");
    Reco_QQ_mumi_TMOneStaTight[Reco_QQ_size] = muon1->muonID("TMOneStationTight");

    iTrack_mupl = muon2->innerTrack();
    iTrack_mumi = muon1->innerTrack();

    if (muon2->isGlobalMuon())
      gTrack_mupl = muon2->globalTrack();

    if (muon1->isGlobalMuon())
      gTrack_mumi = muon1->globalTrack();
  }


  if (!_theMinimumFlag) {
    Reco_QQ_mupl_nTrkHits[Reco_QQ_size] = iTrack_mupl->found();
    Reco_QQ_mumi_nTrkHits[Reco_QQ_size] = iTrack_mumi->found();
    Reco_QQ_mupl_highPurity[Reco_QQ_size] = iTrack_mupl->quality(reco::TrackBase::highPurity);
    Reco_QQ_mumi_highPurity[Reco_QQ_size] = iTrack_mumi->quality(reco::TrackBase::highPurity);
    Reco_QQ_mupl_normChi2_inner[Reco_QQ_size] = iTrack_mupl->normalizedChi2();
    Reco_QQ_mumi_normChi2_inner[Reco_QQ_size] = iTrack_mumi->normalizedChi2();
    Reco_QQ_mupl_nPixValHits[Reco_QQ_size] = iTrack_mupl->hitPattern().numberOfValidPixelHits();
    Reco_QQ_mumi_nPixValHits[Reco_QQ_size] = iTrack_mumi->hitPattern().numberOfValidPixelHits(); 
    Reco_QQ_mupl_nPixWMea[Reco_QQ_size] = iTrack_mupl->hitPattern().pixelLayersWithMeasurement();
    Reco_QQ_mumi_nPixWMea[Reco_QQ_size] = iTrack_mumi->hitPattern().pixelLayersWithMeasurement();
    Reco_QQ_mupl_nTrkWMea[Reco_QQ_size] = iTrack_mupl->hitPattern().trackerLayersWithMeasurement();
    Reco_QQ_mumi_nTrkWMea[Reco_QQ_size] = iTrack_mumi->hitPattern().trackerLayersWithMeasurement();
    Reco_QQ_mupl_dxy[Reco_QQ_size] = iTrack_mupl->dxy(RefVtx);
    Reco_QQ_mumi_dxy[Reco_QQ_size] = iTrack_mumi->dxy(RefVtx);
    Reco_QQ_mupl_dxyErr[Reco_QQ_size] = iTrack_mupl->dxyError();
    Reco_QQ_mumi_dxyErr[Reco_QQ_size] = iTrack_mumi->dxyError();
    Reco_QQ_mupl_dz[Reco_QQ_size] = iTrack_mupl->dz(RefVtx);
    Reco_QQ_mumi_dz[Reco_QQ_size] = iTrack_mumi->dz(RefVtx);
    Reco_QQ_mupl_dzErr[Reco_QQ_size] = iTrack_mupl->dzError();
    Reco_QQ_mumi_dzErr[Reco_QQ_size] = iTrack_mumi->dzError();
    Reco_QQ_mupl_pt_inner[Reco_QQ_size] = iTrack_mupl->pt();
    Reco_QQ_mumi_pt_inner[Reco_QQ_size] = iTrack_mumi->pt();
    Reco_QQ_mupl_ptErr_inner[Reco_QQ_size] = iTrack_mupl->ptError();
    Reco_QQ_mumi_ptErr_inner[Reco_QQ_size] = iTrack_mumi->ptError();
    
    if (gTrack_mupl.isNonnull()) {
      Reco_QQ_mupl_nMuValHits[Reco_QQ_size] = gTrack_mupl->hitPattern().numberOfValidMuonHits();
      Reco_QQ_mupl_normChi2_global[Reco_QQ_size] = gTrack_mupl->normalizedChi2();
      Reco_QQ_mupl_pt_global[Reco_QQ_size] = gTrack_mupl->pt();
      Reco_QQ_mupl_ptErr_global[Reco_QQ_size] = gTrack_mupl->ptError();
    }
    else {
      Reco_QQ_mupl_nMuValHits[Reco_QQ_size] = -1;
      Reco_QQ_mupl_normChi2_global[Reco_QQ_size] = 999;
      Reco_QQ_mupl_pt_global[Reco_QQ_size] = -1;
      Reco_QQ_mupl_ptErr_global[Reco_QQ_size] = -1;
    }

    if (gTrack_mumi.isNonnull()) {
      Reco_QQ_mumi_nMuValHits[Reco_QQ_size] = gTrack_mumi->hitPattern().numberOfValidMuonHits();
      Reco_QQ_mumi_normChi2_global[Reco_QQ_size] = gTrack_mumi->normalizedChi2();
      Reco_QQ_mumi_pt_global[Reco_QQ_size] = gTrack_mumi->pt();
      Reco_QQ_mumi_ptErr_global[Reco_QQ_size] = gTrack_mumi->ptError();
    }
    else {
      Reco_QQ_mumi_nMuValHits[Reco_QQ_size] = -1;
      Reco_QQ_mumi_normChi2_global[Reco_QQ_size] = 999;
      Reco_QQ_mumi_pt_global[Reco_QQ_size] = -1;
      Reco_QQ_mumi_ptErr_global[Reco_QQ_size] = -1;
    }
  }

  
  TLorentzVector vJpsi = lorentzMomentum(aJpsiCand->p4());
  new((*Reco_QQ_4mom)[Reco_QQ_size])TLorentzVector(vJpsi);

  if (_useBS) {
    if (aJpsiCand->hasUserFloat("ppdlBS")) {
      Reco_QQ_ctau[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS");
    } else {  
      Reco_QQ_ctau[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlBS was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlErrBS")) {
      Reco_QQ_ctauErr[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS");
    } else {
      Reco_QQ_ctauErr[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlErrBS was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlBS3D")) {
      Reco_QQ_ctau3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlBS3D");
    } else {
      Reco_QQ_ctau3D[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlBS3D was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlErrBS3D")) {
      Reco_QQ_ctauErr3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrBS3D");
    } else {
      Reco_QQ_ctauErr3D[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlErrBS3D was not found" << std::endl;
    }
  }
  else {
    if (aJpsiCand->hasUserFloat("ppdlPV")) {
      Reco_QQ_ctau[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV");
    } else {
      Reco_QQ_ctau[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlPV was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlErrPV")) {
      Reco_QQ_ctauErr[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV");
    } else {
      Reco_QQ_ctauErr[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlErrPV was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlPV3D")) {
      Reco_QQ_ctau3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlPV3D");
    } else {
      Reco_QQ_ctau3D[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlPV3D was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlErrPV3D")) {
      Reco_QQ_ctauErr3D[Reco_QQ_size] = 10.0*aJpsiCand->userFloat("ppdlErrPV3D");
    } else {
      Reco_QQ_ctau3D[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlErrPV3D was not found" << std::endl;
    }
  }
  if (_isMC){
    if (aJpsiCand->hasUserFloat("ppdlTrue")) {
      Reco_QQ_ctauTrue[Reco_QQ_size] = 10.0*(float)aJpsiCand->userFloat("ppdlTrue");
    } else {
      Reco_QQ_ctauTrue[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlTrue was not found" << std::endl;
    }
    if (aJpsiCand->hasUserFloat("ppdlTrue3D")) {
      Reco_QQ_ctauTrue3D[Reco_QQ_size] = 10.0*(float)aJpsiCand->userFloat("ppdlTrue3D");
    } else {
      Reco_QQ_ctauTrue3D[Reco_QQ_size] = -100;
      std::cout << "Warning: User Float ppdlTrue3D was not found" << std::endl;
    }
  }
  if (aJpsiCand->hasUserFloat("vProb")) {
    Reco_QQ_VtxProb[Reco_QQ_size] = aJpsiCand->userFloat("vProb");
  } else {
    Reco_QQ_VtxProb[Reco_QQ_size] = -1;
    std::cout << "Warning: User Float vProb was not found" << std::endl;
  }
  if (aJpsiCand->hasUserFloat("DCA")) {
    Reco_QQ_dca[Reco_QQ_size] = aJpsiCand->userFloat("DCA");
  } else {
    Reco_QQ_dca[Reco_QQ_size] = -99;
    std::cout << "Warning: User Float DCA was not found" << std::endl;
  }
  if (aJpsiCand->hasUserFloat("MassErr")) {
    Reco_QQ_MassErr[Reco_QQ_size] = aJpsiCand->userFloat("MassErr");
  } else {
    Reco_QQ_MassErr[Reco_QQ_size] = -99;
    std::cout << "Warning: User Float MassErr was not found" << std::endl;
  }
  if (aJpsiCand->hasUserInt("Ntrk")) {
    Reco_QQ_Ntrk[Reco_QQ_size] = aJpsiCand->userInt("Ntrk");
  } else {
    Reco_QQ_Ntrk[Reco_QQ_size] = -1;
    std::cout << "Warning: User Int Ntrk was not found" << std::endl;
  }

  Reco_QQ_NtrkDeltaR03[Reco_QQ_size]=0;
  Reco_QQ_NtrkDeltaR04[Reco_QQ_size]=0;
  Reco_QQ_NtrkDeltaR05[Reco_QQ_size]=0;

  Reco_QQ_NtrkPt02[Reco_QQ_size]=0;
  Reco_QQ_NtrkPt03[Reco_QQ_size]=0;
  Reco_QQ_NtrkPt04[Reco_QQ_size]=0;

  if (_useGeTracks && collTracks.isValid()) {
    for(std::vector<reco::Track>::const_iterator it=collTracks->begin();
        it!=collTracks->end(); ++it) {
      const reco::Track* track = &(*it);        

      double dz = track->dz(RefVtx);
      double dzsigma = sqrt(track->dzError()*track->dzError()+RefVtx_zError*RefVtx_zError);    
      double dxy = track->dxy(RefVtx);
      double dxysigma = sqrt(track->dxyError()*track->dxyError() + RefVtx_xError*RefVtx_yError);
      // to be fixed
      // double dxysigma = sqrt(track->dxyError()*track->dxyError() + RefVtx_xError*RefVtx_xError+RefVtx_yError*RefVtx_yError);
      // std::cout << "original: " << dxysigma
      //                << " better: "  << sqrt( pow(track->dxyError(),2) + pow(RefVtx_xError,2) + pow(RefVtx_yError,2) )
      //                << " ratio: " << dxysigma / sqrt( pow(track->dxyError(),2) + pow(RefVtx_xError,2) + pow(RefVtx_yError,2) )
      //                << " best: "
      //                << std::endl;
      

       if (track->qualityByName("highPurity") &&
           track->pt()>0.2 && fabs(track->eta())<2.4 &&
           track->ptError()/track->pt()<0.1 && 
           fabs(dz/dzsigma)<3.0 && fabs(dxy/dxysigma)<3.0)  {
         
         Reco_QQ_NtrkPt02[Reco_QQ_size]++;
         if (track->pt()>0.3) Reco_QQ_NtrkPt03[Reco_QQ_size]++;
         if (track->pt()>0.4) {
           Reco_QQ_NtrkPt04[Reco_QQ_size]++;

         if (iTrack_mupl->charge()==track->charge()) {
           double Reco_QQ_mupl_NtrkDeltaR = deltaR(iTrack_mupl->eta(), iTrack_mupl->phi(), track->eta(), track->phi());
           double Reco_QQ_mupl_RelDelPt = abs(1.0 - iTrack_mupl->pt()/track->pt());

           if ( Reco_QQ_mupl_NtrkDeltaR<0.001 &&
                Reco_QQ_mupl_RelDelPt<0.001 )
             continue;
         }
         else {
           double Reco_QQ_mumi_NtrkDeltaR = deltaR(iTrack_mumi->eta(), iTrack_mumi->phi(), track->eta(), track->phi());
           double Reco_QQ_mumi_RelDelPt = abs(1.0 - iTrack_mumi->pt()/track->pt());

           if ( Reco_QQ_mumi_NtrkDeltaR<0.001 &&
                Reco_QQ_mumi_RelDelPt<0.001 ) 
             continue;
         }

           double Reco_QQ_NtrkDeltaR = deltaR(aJpsiCand->eta(), aJpsiCand->phi(), track->eta(), track->phi());
           if (Reco_QQ_NtrkDeltaR<0.3)
             Reco_QQ_NtrkDeltaR03[Reco_QQ_size]++;
           if (Reco_QQ_NtrkDeltaR<0.4)
             Reco_QQ_NtrkDeltaR04[Reco_QQ_size]++;
           if (Reco_QQ_NtrkDeltaR<0.5)
             Reco_QQ_NtrkDeltaR05[Reco_QQ_size]++;
         }
       }
    }
  }

  Reco_QQ_size++;
  return;
}

void
HiOniaAnalyzer::fillRecoJpsi(int iSign, int count, std::string trigName, std::string centName) {
  pat::CompositeCandidate* aJpsiCand = _thePassedCands[iSign].at(count)->clone();
  aJpsiCand->addUserInt("centBin",centBin);

  std::string theLabel =  trigName + "_" + centName + "_" + theSign.at(iSign);

  bool isBarrel = false;
  if ( fabs(aJpsiCand->rapidity()) < 1.2) isBarrel = true;

  if (iSign==0 &&
      aJpsiCand->mass() >= JpsiMassMin && aJpsiCand->mass() < JpsiMassMax &&  
      aJpsiCand->pt() >= JpsiPtMin && aJpsiCand->pt() < JpsiPtMax && 
      fabs(aJpsiCand->rapidity()) >= JpsiRapMin && fabs(aJpsiCand->rapidity()) < JpsiRapMax) {
    passedCandidates++;
  }

  if (_fillHistos) {
    if (_combineCategories && _thePassedCats[iSign].at(count)<=Trk_Trk) { // for the moment consider Glb+Glb, GlbTrk+GlbTrk, Trk+Trk
      myRecoJpsiHistos->Fill(aJpsiCand, "All_"+ theLabel);
      if (isBarrel)
        myRecoJpsiHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
      else
        myRecoJpsiHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
    }
    else {
      switch (_thePassedCats[iSign].at(count)) {
      case Glb_Glb:
        myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "All_"+ theLabel);
        if (isBarrel)
          myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
        else
          myRecoJpsiGlbGlbHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
        break;
      case GlbTrk_GlbTrk:
        myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "All_"+ theLabel);
        if (isBarrel)
          myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
        else
          myRecoJpsiGlbTrkHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
        break;
      case Trk_Trk:
        myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "All_"+ theLabel);
        if (isBarrel)
          myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "Barrel_"+ theLabel);
        else
          myRecoJpsiTrkTrkHistos->Fill(aJpsiCand, "EndCap_"+ theLabel);
        break;
      default:
        break;
      }
    }
  }

  this->fillHistosAndDS(_thePassedCats[iSign].at(count), aJpsiCand); 

  return;
}

void
HiOniaAnalyzer::fillHistosAndDS(unsigned int theCat, const pat::CompositeCandidate* aJpsiCand) {

  return;
}

void
HiOniaAnalyzer::checkTriggers(const pat::CompositeCandidate* aJpsiCand) {
  const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon1"));
  const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(aJpsiCand->daughter("muon2"));

  // Trigger passed
  for (unsigned int iTr = 1; iTr<NTRIGGERS; ++iTr) {
    const pat::TriggerObjectStandAloneCollection mu1HLTMatchesFilter = muon1->triggerObjectMatchesByFilter( HLTLastFilters[iTr] );
    const pat::TriggerObjectStandAloneCollection mu2HLTMatchesFilter = muon2->triggerObjectMatchesByFilter( HLTLastFilters[iTr] );
    
    const pat::TriggerObjectStandAloneCollection mu1HLTMatchesPath = muon1->triggerObjectMatchesByPath( theTriggerNames.at(iTr), true, false );
    const pat::TriggerObjectStandAloneCollection mu2HLTMatchesPath = muon2->triggerObjectMatchesByPath( theTriggerNames.at(iTr), true, false );
    
    bool pass1 = false;
    bool pass2 = false;

    pass1 = mu1HLTMatchesFilter.size() > 0;
    pass2 = mu2HLTMatchesFilter.size() > 0;
    
    //    pass1 = mu1HLTMatchesPath.size() > 0;
    //    pass2 = mu2HLTMatchesPath.size() > 0;
    
    if (iTr > NTRIGGERS_DBL) {  // single triggers here
      isTriggerMatched[iTr] = pass1 || pass2;
    } else {        // double triggers here
      isTriggerMatched[iTr] = pass1 && pass2;
    }
  }

  for (unsigned int iTr=1;iTr<NTRIGGERS;++iTr) {
    if (isTriggerMatched[iTr]) {
      // fill event counting histogram only once per event, also if several muons fired trigger
      //      if (alreadyFilled[iTr]) continue;
      // since we have bins for event info, let's try to fill here the trigger info for each pair
      // also if there are several pairs matched to the same kind of trigger
      hStats->Fill(iTr+NTRIGGERS); // pair info
      //      alreadyFilled[iTr]=true;
    }
  }

  return;
}

void
HiOniaAnalyzer::makeCuts(int sign) {
  math::XYZPoint RefVtx_tmp = RefVtx;

  if (collJpsi.isValid()) {
    for(std::vector<pat::CompositeCandidate>::const_iterator it=collJpsi->begin();
        it!=collJpsi->end(); ++it) {
      
      const pat::CompositeCandidate* cand = &(*it);     
      if (fabs(cand->rapidity()) >= etaMax) continue;

      if (!(_isHI || _isPA) && _muonLessPrimaryVertex && cand->hasUserData("muonlessPV"))
        RefVtx = (*cand->userData<reco::Vertex>("muonlessPV")).position();
      else if (!_muonLessPrimaryVertex && cand->hasUserData("PVwithmuons"))
        RefVtx = (*cand->userData<reco::Vertex>("PVwithmuons")).position();
      else {
        std::cout << "HiOniaAnalyzer::makeCuts: no PV for muon pair stored" << std::endl;
        continue;
      }

      if (fabs(RefVtx.Z()) > _iConfig.getParameter< double > ("maxAbsZ")) continue;

      const pat::Muon* muon1 = dynamic_cast<const pat::Muon*>(cand->daughter("muon1"));
      const pat::Muon* muon2 = dynamic_cast<const pat::Muon*>(cand->daughter("muon2"));
      
      if (fabs(muon1->eta()) >= etaMax ||
          fabs(muon2->eta()) >= etaMax) continue;

      bool thisSign = ( (sign == 0 && muon1->charge() + muon2->charge() == 0) || 
                        (sign == 1 && muon1->charge() + muon2->charge() == 2) || 
                        (sign == 2 && muon1->charge() + muon2->charge() == -2) );

      if (thisSign) {
        bool muonSelFound = false;
        if ( _muonSel==(std::string)("Glb") ) {
          if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalMuon,&HiOniaAnalyzer::selGlobalMuon)){
            _thePassedCats[sign].push_back(Glb_Glb);  _thePassedCands[sign].push_back(cand);
          }
          muonSelFound = true;
        }
        if ( _muonSel==(std::string)("GlbTrk") ) {
          if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selGlobalMuon,&HiOniaAnalyzer::selGlobalMuon)){
            _thePassedCats[sign].push_back(GlbTrk_GlbTrk);  _thePassedCands[sign].push_back(cand);
          }
          muonSelFound = true;
        }
        if ( _muonSel==(std::string)("Trk") ) {
          if (checkCuts(cand,muon1,muon2,&HiOniaAnalyzer::selTrackerMuon,&HiOniaAnalyzer::selTrackerMuon)){
            _thePassedCats[sign].push_back(Trk_Trk);  _thePassedCands[sign].push_back(cand);
          }
          muonSelFound = true;
        }
        if (!muonSelFound) {
          std::cout << "[HiOniaAnalyzer::makeCuts] --- The muon selection: " << _muonSel << " is invalid. The supported options are: Glb, GlbTrk and Trk" << std::endl;
        }
      }
    }
  }
  
  RefVtx = RefVtx_tmp;
  return;
}

bool
HiOniaAnalyzer::checkCuts(const pat::CompositeCandidate* cand, const pat::Muon* muon1,  const pat::Muon* muon2, bool(HiOniaAnalyzer::* callFunc1)(const pat::Muon*), bool(HiOniaAnalyzer::* callFunc2)(const pat::Muon*)) {
  if ( (  (this->*callFunc1)(muon1) &&  (this->*callFunc2)(muon2) ) &&
       (!_applycuts || cand->userFloat("vProb") > 0.01) )
    return true;
  else
    return false;
}

pair< unsigned int, const pat::CompositeCandidate* > 
HiOniaAnalyzer::theBestQQ(int sign) {

  unsigned int theBestCat = 99;
  const pat::CompositeCandidate* theBestCand = new pat::CompositeCandidate();

  for( unsigned int i = 0; i < _thePassedCands[sign].size(); i++) { 
    if (_thePassedCats[sign].at(i) < theBestCat) {
      theBestCat = _thePassedCats[sign].at(i);
      theBestCand = _thePassedCands[sign].at(i);
    }
  }

  pair< unsigned int, const pat::CompositeCandidate* > result = make_pair(theBestCat, theBestCand );
  return result;
}

bool
HiOniaAnalyzer::isMuonInAccept(const pat::Muon* aMuon, const std::string muonType) {
  if (muonType == (std::string)("GLB")) {
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 1.2 && aMuon->pt() >= 3.5) ||
             (1.2 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 2.1 && aMuon->pt() >= 5.77-1.89*fabs(aMuon->eta())) ||
             (2.1 <= fabs(aMuon->eta()) && aMuon->pt() >= 1.8)));
  }
  else if (muonType == (std::string)("TRK")) {
    return (fabs(aMuon->eta()) < 2.4 &&
            ((fabs(aMuon->eta()) < 1.3 && aMuon->pt() >= 3.3) ||
             (1.3 <= fabs(aMuon->eta()) && fabs(aMuon->eta()) < 2.2 && aMuon->p() >= 2.9) ||
             (2.2 <= fabs(aMuon->eta()) && aMuon->pt() >= 0.8)));
  }
  else  std::cout << "ERROR: Incorrect Muon Type" << std::endl;
  
  return false;
}

bool
HiOniaAnalyzer::isSoftMuon(const pat::Muon* aMuon) {
  return (
          muon::isGoodMuon(*aMuon, muon::TMOneStationTight) &&
          aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5   &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement()   > 0   &&
          //aMuon->innerTrack()->quality(reco::TrackBase::highPurity) &&
          fabs(aMuon->innerTrack()->dxy(RefVtx)) < 0.3 &&
          fabs(aMuon->innerTrack()->dz(RefVtx)) < 20. 
          );
}

bool
HiOniaAnalyzer::isTightMuon(const pat::Muon* aMuon) {
  return (
          aMuon->isGlobalMuon() &&
          //aMuon->isPFMuon()   &&
          aMuon->globalTrack()->normalizedChi2() < 10. &&
          aMuon->globalTrack()->hitPattern().numberOfValidMuonHits() > 0 &&
          aMuon->numberOfMatchedStations() > 1 &&
          fabs(aMuon->muonBestTrack()->dxy(RefVtx)) < 0.2 &&
          fabs(aMuon->muonBestTrack()->dz(RefVtx)) < 0.5  &&
          aMuon->innerTrack()->hitPattern().numberOfValidPixelHits() > 0 &&
          aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5
          );
}

bool
HiOniaAnalyzer::selGlobalMuon(const pat::Muon* aMuon) {
  
  if(!aMuon->isGlobalMuon())
    return false;

  if( _muonSel==(std::string)("GlbTrk") && !aMuon->isTrackerMuon() )
    return false;
  
  if(!_applycuts)
    return true;
  
  bool isInAcc = isMuonInAccept(aMuon, (std::string)("GLB"));
  bool isGood = (_selTightGlobalMuon ? isTightMuon(aMuon) : isSoftMuon(aMuon));

  return ( isInAcc && isGood );
}

bool 
HiOniaAnalyzer::selTrackerMuon(const pat::Muon* aMuon) {
  
  if(!aMuon->isTrackerMuon())
    return false;

  if(!_applycuts)
    return true;

  bool isInAcc = isMuonInAccept(aMuon, (std::string)("TRK"));
  bool isGood = isSoftMuon(aMuon);

  return ( isInAcc && isGood );
}

void
HiOniaAnalyzer::InitEvent()
{
  for (unsigned int iTr=1;iTr<NTRIGGERS;++iTr) {
    alreadyFilled[iTr]=false;
  }
  HLTriggers = 0;

  nEP = 0;

  _thePassedCats[0].clear();      _thePassedCands[0].clear();
  _thePassedCats[1].clear();      _thePassedCands[1].clear();
  _thePassedCats[2].clear();      _thePassedCands[2].clear();

  Reco_QQ_size = 0;
  Reco_mu_size = 0;
  Reco_trk_size = 0;

  Gen_QQ_size = 0;
  Gen_mu_size = 0;

  Reco_QQ_4mom->Clear();
  Reco_QQ_vtx->Clear();
  Reco_QQ_mupl_4mom->Clear();
  Reco_QQ_mumi_4mom->Clear();
  Reco_mu_4mom->Clear();

  if (_useGeTracks && _fillRecoTracks) {
    Reco_trk_4mom->Clear();
    Reco_trk_vtx->Clear();
  }

  if (_isMC) {
    Gen_QQ_4mom->Clear();
    Gen_QQ_mupl_4mom->Clear();
    Gen_QQ_mumi_4mom->Clear();
    Gen_mu_4mom->Clear();
  }

  for(std::map< std::string, int >::iterator clearIt= mapTriggerNameToIntFired_.begin(); clearIt != mapTriggerNameToIntFired_.end(); clearIt++){
    clearIt->second=0;
  }
  for(std::map< std::string, int >::iterator clearIt= mapTriggerNameToPrescaleFac_.begin(); clearIt != mapTriggerNameToPrescaleFac_.end(); clearIt++){
    clearIt->second=-1;
  }

  return;
}

reco::GenParticleRef  
HiOniaAnalyzer::findDaughterRef(reco::GenParticleRef GenParticleDaughter, int GenParticlePDG) {

  reco::GenParticleRef GenParticleTmp = GenParticleDaughter;   
  for(int j=0; j<1000; ++j) { 
    if (GenParticleTmp.isNonnull() && ((GenParticleTmp->pdgId()==GenParticlePDG) || (GenParticleTmp->pdgId()==GenParticleDaughter->pdgId())) && GenParticleTmp->numberOfDaughters()>0) { 
      GenParticleDaughter = GenParticleTmp;
      GenParticleTmp      = GenParticleDaughter->daughterRef(0);
    } else break;
  }
  if (GenParticleTmp.isNonnull() && (GenParticleTmp->pdgId()==GenParticleDaughter->pdgId())) {
    GenParticleDaughter = GenParticleTmp;
  }   
  return GenParticleDaughter;

}

void
HiOniaAnalyzer::fillGenInfo()
{
  if (Gen_QQ_size >= Max_QQ_size) {
    std::cout << "Too many dimuons: " << Gen_QQ_size << std::endl;
    std::cout << "Maximum allowed: " << Max_QQ_size << std::endl;
    return;
  }

  if (Gen_mu_size >= Max_mu_size) {
    std::cout << "Too many muons: " << Gen_mu_size << std::endl;
    std::cout << "Maximum allowed: " << Max_mu_size << std::endl;
    return;
  }
  
  if (collGenParticles.isValid()) {
    for(std::vector<reco::GenParticle>::const_iterator it=collGenParticles->begin();
        it!=collGenParticles->end();++it) {
      const reco::GenParticle* gen = &(*it);

      if (abs(gen->pdgId()) == _oniaPDG  && gen->status() == 2 &&
          gen->numberOfDaughters() >= 2) {

        reco::GenParticleRef genMuon1 = findDaughterRef(gen->daughterRef(0), gen->pdgId());
        reco::GenParticleRef genMuon2 = findDaughterRef(gen->daughterRef(1), gen->pdgId());

        if ( abs(genMuon1->pdgId()) == 13 &&
             abs(genMuon2->pdgId()) == 13 &&
             genMuon1->status() == 1 &&
             genMuon2->status() == 1 ) {
          
          Gen_QQ_type[Gen_QQ_size] = _isPromptMC ? 0 : 1; // prompt: 0, non-prompt: 1
          std::pair<int, std::pair<float, float> > MCinfo = findGenMCInfo(gen);
          Gen_QQ_momId[Gen_QQ_size] = MCinfo.first;
          Gen_QQ_ctau[Gen_QQ_size] = 10.0*MCinfo.second.first;
          Gen_QQ_ctau3D[Gen_QQ_size] = 10.0*MCinfo.second.second;
          
          TLorentzVector vJpsi = lorentzMomentum(gen->p4());
          new((*Gen_QQ_4mom)[Gen_QQ_size])TLorentzVector(vJpsi);

          TLorentzVector vMuon1 = lorentzMomentum(genMuon1->p4());
          TLorentzVector vMuon2 = lorentzMomentum(genMuon2->p4());
            
          if (genMuon1->charge() > genMuon2->charge()) {
            new((*Gen_QQ_mupl_4mom)[Gen_QQ_size])TLorentzVector(vMuon1);
            new((*Gen_QQ_mumi_4mom)[Gen_QQ_size])TLorentzVector(vMuon2);
          }
          else {
            new((*Gen_QQ_mupl_4mom)[Gen_QQ_size])TLorentzVector(vMuon2);
            new((*Gen_QQ_mumi_4mom)[Gen_QQ_size])TLorentzVector(vMuon1);
          }

          Gen_QQ_size++;
        }
      }

      if (abs(gen->pdgId()) == 13  && gen->status() == 1) {
        Gen_mu_type[Gen_mu_size] = _isPromptMC ? 0 : 1; // prompt: 0, non-prompt: 1
        Gen_mu_charge[Gen_mu_size] = gen->charge();

        TLorentzVector vMuon = lorentzMomentum(gen->p4());
        new((*Gen_mu_4mom)[Gen_mu_size])TLorentzVector(vMuon);

        Gen_mu_size++;
      }

    }
  }
  
  return;
}

void
HiOniaAnalyzer::fillRecoTracks()
{
  if (collTracks.isValid()) {
    for(std::vector<reco::Track>::const_iterator it=collTracks->begin();
        it!=collTracks->end(); ++it) {
      const reco::Track* track = &(*it);        

      // double dz = track->dz(RefVtx);
      // double dzsigma = sqrt(track->dzError()*track->dzError()+RefVtx_zError*RefVtx_zError);    
      // double dxy = track->dxy(RefVtx);
      // double dxysigma = sqrt(track->dxyError()*track->dxyError() + RefVtx_xError*RefVtx_yError);
      
      if (track->qualityByName("highPurity") &&
          track->pt()>0.2 && fabs(track->eta())<2.4 &&
          track->ptError()/track->pt()<0.1)  {
        if (Reco_trk_size >= Max_trk_size) {
          std::cout << "Too many tracks: " << Reco_trk_size << std::endl;
          std::cout << "Maximum allowed: " << Max_trk_size << std::endl;
          break;
        }
        
        Reco_trk_charge[Reco_trk_size] = track->charge();
         
        new((*Reco_trk_vtx)[Reco_trk_size])TVector3(track->vx(),track->vy(),track->vz());

        Reco_trk_dxyError[Reco_trk_size] = track->dxyError();
        Reco_trk_dzError[Reco_trk_size] = track->dzError();

        TLorentzVector vTrack;
        vTrack.SetPtEtaPhiM(track->pt(), track->eta(), track->phi(), 0.13957018);
        new((*Reco_trk_4mom)[Reco_trk_size])TLorentzVector(vTrack);
        Reco_trk_size++;
      }
    }
  }
  
  return;
}

void
HiOniaAnalyzer::fillRecoMuons(int iCent)
{
  int nL1DoubleMu0Muons=0;
  int nGoodMuons=0;
  int nGoodMuonsNoTrig=0;

  if (collMuonNoTrig.isValid()) {
    for(std::vector<pat::Muon>::const_iterator it=collMuonNoTrig->begin();
        it!=collMuonNoTrig->end();++it) {
      const pat::Muon* muon = &(*it);

      if (muon->isGlobalMuon() &&
          selGlobalMuon(muon))
        nGoodMuonsNoTrig++;
    }
  }

  if (collMuon.isValid()) {
    for(vector<pat::Muon>::const_iterator it=collMuon->begin();
        it!=collMuon->end();++it) {
      const pat::Muon* muon = &(*it);

      bool isBarrel = false;
      if ( fabs(muon->eta() < 1.2) ) isBarrel = true;
      std::string theLabel = theTriggerNames.at(0) + "_" + theCentralities.at(iCent);

      if (_fillHistos) {
        if (_combineCategories) {
          if ( (muon->isGlobalMuon() &&
                selGlobalMuon(muon)) ||
               (muon->isTrackerMuon() &&
                selTrackerMuon(muon)) ) {
            myRecoMuonHistos->Fill(muon, "All_"+theLabel);
            if (isBarrel)
              myRecoMuonHistos->Fill(muon, "Barrel_"+theLabel);
            else
              myRecoMuonHistos->Fill(muon, "EndCap_"+theLabel);
          }
        }
        else {
          if (muon->isGlobalMuon() &&
              selGlobalMuon(muon)) {
          
            myRecoGlbMuonHistos->Fill(muon, "All_"+theLabel);
            if (isBarrel)
              myRecoGlbMuonHistos->Fill(muon, "Barrel_"+theLabel);
            else
              myRecoGlbMuonHistos->Fill(muon, "EndCap_"+theLabel);
          }
          else if (muon->isTrackerMuon() &&
                   selTrackerMuon(muon)) {
            myRecoTrkMuonHistos->Fill(muon, "All_"+theLabel);
            if (isBarrel)
              myRecoTrkMuonHistos->Fill(muon, "Barrel_"+theLabel);
            else
              myRecoTrkMuonHistos->Fill(muon, "EndCap_"+theLabel);
          }
        }
      }
      
      muType = -99;
      if ( _muonSel==(std::string)("Glb")    && selGlobalMuon(muon)  ) muType = Glb;
      if ( _muonSel==(std::string)("GlbTrk") && selGlobalMuon(muon)  ) muType = GlbTrk;
      if ( _muonSel==(std::string)("Trk")    && selTrackerMuon(muon) ) muType = Trk; 
      
      if ( muType==Glb || muType==GlbTrk || muType==Trk ) {
        nGoodMuons++;

        ULong64_t trigBits=0;
        for (unsigned int iTr=1; iTr<NTRIGGERS; ++iTr) {
          const pat::TriggerObjectStandAloneCollection muHLTMatchesFilter = muon->triggerObjectMatchesByFilter(  HLTLastFilters[iTr] );
          const pat::TriggerObjectStandAloneCollection muHLTMatchesPath = muon->triggerObjectMatchesByPath( theTriggerNames.at(iTr), true, false );

          // apparently matching by path gives false positives so we use matching by filter for all triggers for which we know the filter name
          if ( muHLTMatchesFilter.size() > 0 ) {
            std::string theLabel = theTriggerNames.at(iTr) + "_" + theCentralities.at(iCent);
            
            if (_fillHistos) {
              if (_combineCategories) {
                myRecoMuonHistos->Fill(muon, "All_"+theLabel);
                if (isBarrel)
                  myRecoMuonHistos->Fill(muon, "Barrel_"+theLabel);
                else
                  myRecoMuonHistos->Fill(muon, "EndCap_"+theLabel);
              }
              else if ( muType==Glb || muType==GlbTrk ) {
                myRecoGlbMuonHistos->Fill(muon, "All_"+theLabel);
                if (isBarrel)
                  myRecoGlbMuonHistos->Fill(muon, "Barrel_"+theLabel);
                else
                  myRecoGlbMuonHistos->Fill(muon, "EndCap_"+theLabel);
              }
              else if ( muType==Trk ) {
                myRecoTrkMuonHistos->Fill(muon, "All_"+theLabel);
                if (isBarrel)
                  myRecoTrkMuonHistos->Fill(muon, "Barrel_"+theLabel);
                else
                  myRecoTrkMuonHistos->Fill(muon, "EndCap_"+theLabel);
              }
            }

            trigBits += pow(2,iTr-1);

            if (iTr==1) nL1DoubleMu0Muons++;
          }
        }
        if (_fillTree)
          this->fillTreeMuon(muon, muType, trigBits);
      }
    }
  }
  
  hGoodMuonsNoTrig->Fill(nGoodMuonsNoTrig);
  hGoodMuons->Fill(nGoodMuons);
  hL1DoubleMu0->Fill(nL1DoubleMu0Muons);

  return;
}

void
HiOniaAnalyzer::InitTree()
{

  Reco_mu_4mom = new TClonesArray("TLorentzVector", 100);
  Reco_QQ_4mom = new TClonesArray("TLorentzVector",10);
  Reco_QQ_mupl_4mom = new TClonesArray("TLorentzVector",10);
  Reco_QQ_mumi_4mom = new TClonesArray("TLorentzVector",10);

  Reco_QQ_vtx = new TClonesArray("TVector3", 100);

  if (_useGeTracks && _fillRecoTracks) {
    Reco_trk_4mom = new TClonesArray("TLorentzVector", 100);
    Reco_trk_vtx = new TClonesArray("TVector3", 100);
  }

  if (_isMC) {
    Gen_mu_4mom = new TClonesArray("TLorentzVector", 2);
    Gen_QQ_4mom = new TClonesArray("TLorentzVector", 2);
    Gen_QQ_mupl_4mom = new TClonesArray("TLorentzVector", 2);
    Gen_QQ_mumi_4mom = new TClonesArray("TLorentzVector", 2);
  }

  //myTree = new TTree("myTree","My TTree of dimuons");
  myTree = fs->make<TTree>("myTree","My TTree of dimuons");
  
  myTree->Branch("eventNb", &eventNb,   "eventNb/i");
  myTree->Branch("runNb",   &runNb,     "runNb/i");
  myTree->Branch("LS",      &lumiSection, "LS/i"); 
  myTree->Branch("zVtx",    &zVtx,        "zVtx/F"); 
  myTree->Branch("nPV",    &nPV,        "nPV/F"); 
  myTree->Branch("Centrality", &centBin, "Centrality/I");

  myTree->Branch("nTrig", &nTrig, "nTrig/I");
  myTree->Branch("trigPrescale", trigPrescale, "trigPrescale[nTrig]/I");
  myTree->Branch("HLTriggers", &HLTriggers, "HLTriggers/l");

  myTree->Branch("Npix",&Npix,"Npix/I");
  myTree->Branch("NpixelTracks",&NpixelTracks,"NpixelTracks/I");
  myTree->Branch("Ntracks", &Ntracks, "Ntracks/I");
  myTree->Branch("SumET_HF",&SumET_HF,"SumET_HF/F");
  myTree->Branch("SumET_HFplus",&SumET_HFplus,"SumET_HFplus/F");
  myTree->Branch("SumET_HFminus",&SumET_HFminus,"SumET_HFminus/F");
  myTree->Branch("SumET_HFplusEta4",&SumET_HFplusEta4,"SumET_HFplusEta4/F");
  myTree->Branch("SumET_HFminusEta4",&SumET_HFminusEta4,"SumET_HFminusEta4/F");
  myTree->Branch("SumET_ET",&SumET_ET,"SumET_ET/F");
  myTree->Branch("SumET_EE",&SumET_EE,"SumET_EE/F");
  myTree->Branch("SumET_EB",&SumET_EB,"SumET_EB/F");
  myTree->Branch("SumET_EEplus",&SumET_EEplus,"SumET_EEplus/F");
  myTree->Branch("SumET_EEminus",&SumET_EEminus,"SumET_EEminus/F");
  myTree->Branch("SumET_ZDC",&SumET_ZDC,"SumET_ZDC/F");
  myTree->Branch("SumET_ZDCplus",&SumET_ZDCplus,"SumET_ZDCplus/F");
  myTree->Branch("SumET_ZDCminus",&SumET_ZDCminus,"SumET_ZDCminus/F");

  if (_isHI && _useEvtPlane) {
    myTree->Branch("nEP", &nEP, "nEP/I");
    myTree->Branch("rpAng", &rpAng, "rpAng[nEP]/F");
    myTree->Branch("rpSin", &rpSin, "rpSin[nEP]/F");
    myTree->Branch("rpCos", &rpCos, "rpCos[nEP]/F");
  }

  myTree->Branch("Reco_QQ_size", &Reco_QQ_size,  "Reco_QQ_size/I");
  myTree->Branch("Reco_QQ_type", Reco_QQ_type,   "Reco_QQ_type[Reco_QQ_size]/I");
  myTree->Branch("Reco_QQ_sign", Reco_QQ_sign,   "Reco_QQ_sign[Reco_QQ_size]/I");
  myTree->Branch("Reco_QQ_4mom", "TClonesArray", &Reco_QQ_4mom, 32000, 0);
  myTree->Branch("Reco_QQ_mupl_4mom", "TClonesArray", &Reco_QQ_mupl_4mom, 32000, 0);
  myTree->Branch("Reco_QQ_mumi_4mom", "TClonesArray", &Reco_QQ_mumi_4mom, 32000, 0);
  myTree->Branch("Reco_QQ_trig", Reco_QQ_trig,   "Reco_QQ_trig[Reco_QQ_size]/l");
  myTree->Branch("Reco_QQ_mupl_trig", Reco_QQ_mupl_trig,   "Reco_QQ_mupl_trig[Reco_QQ_size]/l");
  myTree->Branch("Reco_QQ_mumi_trig", Reco_QQ_mumi_trig,   "Reco_QQ_mumi_trig[Reco_QQ_size]/l");
  myTree->Branch("Reco_QQ_isCowboy", Reco_QQ_isCowboy,   "Reco_QQ_isCowboy[Reco_QQ_size]/O");
  myTree->Branch("Reco_QQ_ctau", Reco_QQ_ctau,   "Reco_QQ_ctau[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_ctauErr", Reco_QQ_ctauErr,   "Reco_QQ_ctauErr[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_ctau3D", Reco_QQ_ctau3D,   "Reco_QQ_ctau3D[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_ctauErr3D", Reco_QQ_ctauErr3D,   "Reco_QQ_ctauErr3D[Reco_QQ_size]/F");

  if (_isMC){
    myTree->Branch("Reco_QQ_ctauTrue", Reco_QQ_ctauTrue,   "Reco_QQ_ctauTrue[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_ctauTrue3D", Reco_QQ_ctauTrue3D,   "Reco_QQ_ctauTrue3D[Reco_QQ_size]/F");
  }
  myTree->Branch("Reco_QQ_VtxProb", Reco_QQ_VtxProb,   "Reco_QQ_VtxProb[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_dca", Reco_QQ_dca,   "Reco_QQ_dca[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_MassErr", Reco_QQ_MassErr,   "Reco_QQ_MassErr[Reco_QQ_size]/F");
  myTree->Branch("Reco_QQ_vtx", "TClonesArray", &Reco_QQ_vtx, 32000, 0);
  myTree->Branch("Reco_QQ_Ntrk", Reco_QQ_Ntrk, "Reco_QQ_Ntrk[Reco_QQ_size]/I");

  if (!_theMinimumFlag) {
    myTree->Branch("Reco_QQ_mupl_SelectionType", Reco_QQ_mupl_SelectionType,   "Reco_QQ_mupl_SelectionType[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_SelectionType", Reco_QQ_mumi_SelectionType,   "Reco_QQ_mumi_SelectionType[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_isGoodMuon", Reco_QQ_mupl_isGoodMuon,   "Reco_QQ_mupl_isGoodMuon[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mumi_isGoodMuon", Reco_QQ_mumi_isGoodMuon,   "Reco_QQ_mumi_isGoodMuon[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mupl_highPurity", Reco_QQ_mupl_highPurity,   "Reco_QQ_mupl_highPurity[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mumi_highPurity", Reco_QQ_mumi_highPurity,   "Reco_QQ_mumi_highPurity[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mupl_TrkMuArb", Reco_QQ_mupl_TrkMuArb,   "Reco_QQ_mupl_TrkMuArb[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mumi_TrkMuArb", Reco_QQ_mumi_TrkMuArb,   "Reco_QQ_mumi_TrkMuArb[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mupl_TMOneStaTight", Reco_QQ_mupl_TMOneStaTight, "Reco_QQ_mupl_TMOneStaTight[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mumi_TMOneStaTight", Reco_QQ_mumi_TMOneStaTight, "Reco_QQ_mumi_TMOneStaTight[Reco_QQ_size]/O");
    myTree->Branch("Reco_QQ_mupl_nPixValHits", Reco_QQ_mupl_nPixValHits,   "Reco_QQ_mupl_nPixValHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_nPixValHits", Reco_QQ_mumi_nPixValHits,   "Reco_QQ_mumi_nPixValHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_nMuValHits", Reco_QQ_mupl_nMuValHits,   "Reco_QQ_mupl_nMuValHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_nMuValHits", Reco_QQ_mumi_nMuValHits,   "Reco_QQ_mumi_nMuValHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_nTrkHits",Reco_QQ_mupl_nTrkHits, "Reco_QQ_mupl_nTrkHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_nTrkHits",Reco_QQ_mumi_nTrkHits, "Reco_QQ_mumi_nTrkHits[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_normChi2_inner",Reco_QQ_mupl_normChi2_inner, "Reco_QQ_mupl_normChi2_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_normChi2_inner",Reco_QQ_mumi_normChi2_inner, "Reco_QQ_mumi_normChi2_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_normChi2_global",Reco_QQ_mupl_normChi2_global, "Reco_QQ_mupl_normChi2_global[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_normChi2_global",Reco_QQ_mumi_normChi2_global, "Reco_QQ_mumi_normChi2_global[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_nPixWMea",Reco_QQ_mupl_nPixWMea, "Reco_QQ_mupl_nPixWMea[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_nPixWMea",Reco_QQ_mumi_nPixWMea, "Reco_QQ_mumi_nPixWMea[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_nTrkWMea",Reco_QQ_mupl_nTrkWMea, "Reco_QQ_mupl_nTrkWMea[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_nTrkWMea",Reco_QQ_mumi_nTrkWMea, "Reco_QQ_mumi_nTrkWMea[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_StationsMatched",Reco_QQ_mupl_StationsMatched, "Reco_QQ_mupl_StationsMatched[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mumi_StationsMatched",Reco_QQ_mumi_StationsMatched, "Reco_QQ_mumi_StationsMatched[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_mupl_dxy",Reco_QQ_mupl_dxy, "Reco_QQ_mupl_dxy[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_dxy",Reco_QQ_mumi_dxy, "Reco_QQ_mumi_dxy[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_dxyErr",Reco_QQ_mupl_dxyErr, "Reco_QQ_mupl_dxyErr[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_dxyErr",Reco_QQ_mumi_dxyErr, "Reco_QQ_mumi_dxyErr[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_dz",Reco_QQ_mupl_dz, "Reco_QQ_mupl_dz[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_dz",Reco_QQ_mumi_dz, "Reco_QQ_mumi_dz[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_dzErr",Reco_QQ_mupl_dzErr, "Reco_QQ_mupl_dzErr[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_dzErr",Reco_QQ_mumi_dzErr, "Reco_QQ_mumi_dzErr[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_pt_inner",Reco_QQ_mupl_pt_inner, "Reco_QQ_mupl_pt_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_pt_inner",Reco_QQ_mumi_pt_inner, "Reco_QQ_mumi_pt_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_pt_global",Reco_QQ_mupl_pt_global, "Reco_QQ_mupl_pt_global[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_pt_global",Reco_QQ_mumi_pt_global, "Reco_QQ_mumi_pt_global[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_ptErr_inner",Reco_QQ_mupl_ptErr_inner, "Reco_QQ_mupl_ptErr_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_ptErr_inner",Reco_QQ_mumi_ptErr_inner, "Reco_QQ_mumi_ptErr_inner[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mupl_ptErr_global",Reco_QQ_mupl_ptErr_global, "Reco_QQ_mupl_ptErr_global[Reco_QQ_size]/F");
    myTree->Branch("Reco_QQ_mumi_ptErr_global",Reco_QQ_mumi_ptErr_global, "Reco_QQ_mumi_ptErr_global[Reco_QQ_size]/F");
  }

  myTree->Branch("Reco_mu_size", &Reco_mu_size,  "Reco_mu_size/I");
  myTree->Branch("Reco_mu_type", Reco_mu_type,   "Reco_mu_type[Reco_mu_size]/I");
  myTree->Branch("Reco_mu_SelectionType", Reco_mu_SelectionType,   "Reco_mu_SelectionType[Reco_mu_size]/I");
  myTree->Branch("Reco_mu_charge", Reco_mu_charge,   "Reco_mu_charge[Reco_mu_size]/I");
  myTree->Branch("Reco_mu_4mom", "TClonesArray", &Reco_mu_4mom, 32000, 0);
  myTree->Branch("Reco_mu_trig", Reco_mu_trig,   "Reco_mu_trig[Reco_mu_size]/l");

  if (_useGeTracks && _fillRecoTracks) {
    myTree->Branch("Reco_QQ_NtrkPt02", Reco_QQ_NtrkPt02, "Reco_QQ_NtrkPt02[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_NtrkPt03", Reco_QQ_NtrkPt03, "Reco_QQ_NtrkPt03[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_NtrkPt04", Reco_QQ_NtrkPt04, "Reco_QQ_NtrkPt04[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_NtrkDeltaR03", Reco_QQ_NtrkDeltaR03, "Reco_QQ_NtrkDeltaR03[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_NtrkDeltaR04", Reco_QQ_NtrkDeltaR04, "Reco_QQ_NtrkDeltaR04[Reco_QQ_size]/I");
    myTree->Branch("Reco_QQ_NtrkDeltaR05", Reco_QQ_NtrkDeltaR05, "Reco_QQ_NtrkDeltaR05[Reco_QQ_size]/I");

    myTree->Branch("Reco_trk_size", &Reco_trk_size,  "Reco_trk_size/I");
    myTree->Branch("Reco_trk_charge", Reco_trk_charge,   "Reco_trk_charge[Reco_trk_size]/I");
    myTree->Branch("Reco_trk_4mom", "TClonesArray", &Reco_trk_4mom, 32000, 0);
    myTree->Branch("Reco_trk_vtx", "TClonesArray", &Reco_trk_vtx, 32000, 0);
    myTree->Branch("Reco_trk_dxyError", Reco_trk_dxyError, "Reco_trk_dxyError[Reco_trk_size]/F");
    myTree->Branch("Reco_trk_dzError", Reco_trk_dzError, "Reco_trk_dzError[Reco_trk_size]/F");
  }

  if (!_theMinimumFlag) {
    myTree->Branch("Reco_mu_isGoodMuon", Reco_mu_isGoodMuon,   "Reco_mu_isGoodMuon[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_highPurity", Reco_mu_highPurity,   "Reco_mu_highPurity[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_TrkMuArb", Reco_mu_TrkMuArb,   "Reco_mu_TrkMuArb[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_TMOneStaTight", Reco_mu_TMOneStaTight, "Reco_mu_TMOneStaTight[Reco_mu_size]/O");
    myTree->Branch("Reco_mu_nPixValHits", Reco_mu_nPixValHits,   "Reco_mu_nPixValHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nMuValHits", Reco_mu_nMuValHits,   "Reco_mu_nMuValHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nTrkHits",Reco_mu_nTrkHits, "Reco_mu_nTrkHits[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_normChi2_inner",Reco_mu_normChi2_inner, "Reco_mu_normChi2_inner[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_normChi2_global",Reco_mu_normChi2_global, "Reco_mu_normChi2_global[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_nPixWMea",Reco_mu_nPixWMea, "Reco_mu_nPixWMea[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_nTrkWMea",Reco_mu_nTrkWMea, "Reco_mu_nTrkWMea[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_StationsMatched",Reco_mu_StationsMatched, "Reco_mu_StationsMatched[Reco_mu_size]/I");
    myTree->Branch("Reco_mu_dxy",Reco_mu_dxy, "Reco_mu_dxy[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dxyErr",Reco_mu_dxyErr, "Reco_mu_dxyErr[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dz",Reco_mu_dz, "Reco_mu_dz[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_dzErr",Reco_mu_dzErr, "Reco_mu_dzErr[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_pt_inner",Reco_mu_pt_inner, "Reco_mu_pt_inner[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_pt_global",Reco_mu_pt_global, "Reco_mu_pt_global[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_ptErr_inner",Reco_mu_ptErr_inner, "Reco_mu_ptErr_inner[Reco_mu_size]/F");
    myTree->Branch("Reco_mu_ptErr_global",Reco_mu_ptErr_global, "Reco_mu_ptErr_global[Reco_mu_size]/F");
  }

  if (_isMC) {
    myTree->Branch("Gen_QQ_size",      &Gen_QQ_size,    "Gen_QQ_size/I");
    myTree->Branch("Gen_QQ_type",      Gen_QQ_type,    "Gen_QQ_type[Gen_QQ_size]/I");
    myTree->Branch("Gen_QQ_4mom",      "TClonesArray", &Gen_QQ_4mom, 32000, 0);
    myTree->Branch("Gen_QQ_momId",      Gen_QQ_momId,    "Gen_QQ_momId[Gen_QQ_size]/I");
    myTree->Branch("Gen_QQ_ctau",      Gen_QQ_ctau,    "Gen_QQ_ctau[Gen_QQ_size]/F");
    myTree->Branch("Gen_QQ_ctau3D",      Gen_QQ_ctau3D,    "Gen_QQ_ctau3D[Gen_QQ_size]/F");  
    myTree->Branch("Gen_QQ_mupl_4mom", "TClonesArray", &Gen_QQ_mupl_4mom, 32000, 0); 
    myTree->Branch("Gen_QQ_mumi_4mom", "TClonesArray", &Gen_QQ_mumi_4mom, 32000, 0);

    myTree->Branch("Gen_mu_size",   &Gen_mu_size,  "Gen_mu_size/I");
    myTree->Branch("Gen_mu_type",   Gen_mu_type,   "Gen_mu_type[Gen_mu_size]/I");
    myTree->Branch("Gen_mu_charge", Gen_mu_charge, "Gen_mu_charge[Gen_mu_size]/I");
    myTree->Branch("Gen_mu_4mom",   "TClonesArray", &Gen_mu_4mom, 32000, 0);
  }

  return;
}

// ------------ method called once each job just before starting event loop  ------------
void 
HiOniaAnalyzer::beginJob()
{
  //fOut = new TFile(_histfilename.c_str(), "RECREATE");
  InitTree();

  // book histos
  //hGoodMuonsNoTrig = new TH1F("hGoodMuonsNoTrig","hGoodMuonsNoTrig",10,0,10);
  hGoodMuonsNoTrig = fs->make<TH1F>("hGoodMuonsNoTrig","hGoodMuonsNoTrig",10,0,10);
  //hGoodMuons = new TH1F("hGoodMuons","hGoodMuons",10,0,10);
  hGoodMuons = fs->make<TH1F>("hGoodMuons","hGoodMuons",10,0,10);
  //hL1DoubleMu0 = new TH1F("hL1DoubleMu0","hL1DoubleMu0",10,0,10);
  hL1DoubleMu0 = fs->make<TH1F>("hL1DoubleMu0","hL1DoubleMu0",10,0,10);
  
  hGoodMuonsNoTrig->Sumw2();
  hGoodMuons->Sumw2();
  hL1DoubleMu0->Sumw2();

  // muons
  if (_combineCategories) 
    myRecoMuonHistos = new MyCommonHistoManager("RecoMuon");
  else {
    myRecoGlbMuonHistos = new MyCommonHistoManager("GlobalMuon");
    myRecoTrkMuonHistos = new MyCommonHistoManager("TrackerMuon");
  }

  // J/psi
  if (_combineCategories)
    myRecoJpsiHistos = new MyCommonHistoManager("RecoJpsi");
  else {
    myRecoJpsiGlbGlbHistos = new MyCommonHistoManager("GlbGlbJpsi");
    myRecoJpsiGlbTrkHistos = new MyCommonHistoManager("GlbTrkJpsi");
    myRecoJpsiTrkTrkHistos = new MyCommonHistoManager("TrkTrkJpsi");
  }
  
  for (unsigned int i=0; i<theRegions.size(); ++i) {
    for (unsigned int j=0; j<NTRIGGERS; ++j) {
      for (unsigned int k=0; k<theCentralities.size(); ++k) {

        std::string theAppendix = theRegions.at(i) ;
        theAppendix += "_" + theTriggerNames.at(j);
        theAppendix += "_" + theCentralities.at(k);

        // muons
        if (_combineCategories) {
          myRecoMuonHistos->Add(theAppendix);
          myRecoMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
        }
        else {
          myRecoGlbMuonHistos->Add(theAppendix);
          myRecoTrkMuonHistos->Add(theAppendix);
          
          myRecoGlbMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoGlbMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
          
          myRecoTrkMuonHistos->GetHistograms(theAppendix)->SetMassBinning(1,0.10,0.11);
          myRecoTrkMuonHistos->GetHistograms(theAppendix)->SetPtBinning(200,0.0,100.0);
        }

        for (unsigned int l=0; l<theSign.size(); ++l) {
          // J/psi
          if (_combineCategories)
            myRecoJpsiHistos->Add(theAppendix + "_" + theSign.at(l));
          else {
            myRecoJpsiGlbGlbHistos->Add(theAppendix + "_" + theSign.at(l));
            myRecoJpsiGlbTrkHistos->Add(theAppendix + "_" + theSign.at(l));
            myRecoJpsiTrkTrkHistos->Add(theAppendix + "_" + theSign.at(l));
          }
        }
      }
    }
  }

  if (_combineCategories)
    myRecoMuonHistos->Print();
  else
    myRecoGlbMuonHistos->Print();

  //hStats = new TH1F("hStats","hStats;;Number of Events",2*NTRIGGERS+1,0,2*NTRIGGERS+1);
  hStats = fs->make<TH1F>("hStats","hStats;;Number of Events",2*NTRIGGERS+1,0,2*NTRIGGERS+1);
  hStats->GetXaxis()->SetBinLabel(1,"All");
  for (int i=2; i< (int) theTriggerNames.size()+1; ++i) {
    hStats->GetXaxis()->SetBinLabel(i,theTriggerNames.at(i-1).c_str()); // event info
    hStats->GetXaxis()->SetBinLabel(i+NTRIGGERS,theTriggerNames.at(i-1).c_str()); // muon pair info
  }
  hStats->Sumw2();

  //hCent = new TH1F("hCent","hCent;centrality bin;Number of Events",200,0,200);
  hCent = fs->make<TH1F>("hCent","hCent;centrality bin;Number of Events",200,0,200);
  hCent->Sumw2();

  //hPileUp = new TH1F("hPileUp","Number of Primary Vertices;n_{PV};counts", 50, 0, 50);
  hPileUp = fs->make<TH1F>("hPileUp","Number of Primary Vertices;n_{PV};counts", 50, 0, 50);
  hPileUp->Sumw2();

  //hZVtx = new TH1F("hZVtx","Primary z-vertex distribution;z_{vtx} [cm];counts", 120, -30, 30);
  hZVtx = fs->make<TH1F>("hZVtx","Primary z-vertex distribution;z_{vtx} [cm];counts", 120, -30, 30);
  hZVtx->Sumw2();

  return;
}

void 
HiOniaAnalyzer::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  //init HLTConfigProvider
  
  EDConsumerBase::Labels labelTriggerResults;
  EDConsumerBase::labelsForToken(_tagTriggerResultsToken, labelTriggerResults);	
  const std::string pro = labelTriggerResults.process;
  bool changed = true;

  //bool init(const edm::Run& iRun, const edm::EventSetup& iSetup, const std::string& processName, bool& changed);
  hltConfigInit = false;
  if( hltConfig.init(iRun, iSetup, pro, changed) ) hltConfigInit = true;

  hltPrescaleInit = false;
  if( hltPrescaleProvider.init(iRun, iSetup, pro, changed) ) hltPrescaleInit = true;

  return;
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HiOniaAnalyzer::endJob() {
  std::cout << "Total number of events = " << nEvents << std::endl;
  std::cout << "Total number of passed candidates = " << passedCandidates << std::endl;
  /*
  fOut->cd();
  hStats->Write();
  hCent->Write();
  hPileUp->Write();
  hZVtx->Write();

  if (_fillTree)  
    myTree->Write();

  hGoodMuonsNoTrig->Write();
  hGoodMuons->Write();
  hL1DoubleMu0->Write();

  if (_fillHistos) {
    // muons
    if (_combineCategories)
      myRecoGlbMuonHistos->Write(fOut);
    else {
      myRecoGlbMuonHistos->Write(fOut);
      myRecoTrkMuonHistos->Write(fOut);
    }

    // J/psi
    if (_combineCategories)
      myRecoJpsiHistos->Write(fOut);
    else {
      myRecoJpsiGlbGlbHistos->Write(fOut);
      myRecoJpsiGlbTrkHistos->Write(fOut);
      myRecoJpsiTrkTrkHistos->Write(fOut);
    }
  }
  */
  return;
}

TLorentzVector
HiOniaAnalyzer::lorentzMomentum(const reco::Candidate::LorentzVector& p) {
  TLorentzVector res;
  res.SetPtEtaPhiM(p.pt(), p.eta(), p.phi(), p.mass());

  return res;
}

void
HiOniaAnalyzer::hltReport(const edm::Event &iEvent ,const edm::EventSetup& iSetup)
{
  std::map<std::string, bool> mapTriggernameToTriggerFired;
  std::map<std::string, unsigned int> mapTriggernameToHLTbit;

  for(std::vector<std::string>::const_iterator it=theTriggerNames.begin(); it !=theTriggerNames.end(); ++it){
    mapTriggernameToTriggerFired[*it]=false;
    mapTriggernameToHLTbit[*it]=1000;
  }

  // HLTConfigProvider
  if ( hltConfigInit ) {
    //! Use HLTConfigProvider
    const unsigned int n= hltConfig.size();
    for (std::map<std::string, unsigned int>::iterator it = mapTriggernameToHLTbit.begin(); it != mapTriggernameToHLTbit.end(); it++) {
      unsigned int triggerIndex= hltConfig.triggerIndex( it->first );
      if (it->first == "NoTrigger") continue;
      if (triggerIndex >= n) {
              std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " not available in config!" << std::endl;
      }
      else {
        it->second= triggerIndex;
        //      std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << it->first << " available in config!" << std::endl;
      }
    }
  }
    
  // Get Trigger Results
  try {
    iEvent.getByToken( _tagTriggerResultsToken, collTriggerResults );
    //    std::cout << "[HiOniaAnalyzer::hltReport] --- J/psi TriggerResult is present in current event" << std::endl;
  }
  catch(...) {
    //    std::cout << "[HiOniaAnalyzer::hltReport] --- J/psi TriggerResults NOT present in current event" << std::endl;
  }
  if ( collTriggerResults.isValid() && (collTriggerResults->size()==hltConfig.size()) ){
    //    std::cout << "[HiOniaAnalyzer::hltReport] --- J/psi TriggerResults IS valid in current event" << std::endl;
      
    // loop over Trigger Results to check if paths was fired
    for(std::vector< std::string >::iterator itHLTNames= theTriggerNames.begin(); itHLTNames != theTriggerNames.end(); itHLTNames++){
      const std::string triggerPathName =  *itHLTNames;
      if ( mapTriggernameToHLTbit[triggerPathName] < 1000 ) {
        if (collTriggerResults->accept( mapTriggernameToHLTbit[triggerPathName] ) ){
          mapTriggerNameToIntFired_[triggerPathName] = 3;
        }
        //-------prescale factor------------
        if ( hltPrescaleInit && hltPrescaleProvider.prescaleSet(iEvent,iSetup)>=0 ) {
          std::pair<std::vector<std::pair<std::string,int> >,int> detailedPrescaleInfo = hltPrescaleProvider.prescaleValuesInDetail(iEvent, iSetup, triggerPathName);
          //get HLT prescale info from hltPrescaleProvider     
          const int hltPrescale = detailedPrescaleInfo.second;
          //get L1 prescale info from hltPrescaleProvider
          int l1Prescale = 1;     
          if (detailedPrescaleInfo.first.size()==1) {
            l1Prescale = detailedPrescaleInfo.first.at(0).second;
          }
          else if (detailedPrescaleInfo.first.size()>1) {
            std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerName " << triggerPathName << " has complex L1 seed " << hltConfig.hltL1GTSeeds(triggerPathName).at(0).second << std::endl;
            std::cout << "[HiOniaAnalyzer::hltReport] --- Need to define a proper way to compute the total L1 prescale, default L1 prescale value set to 1 "  << std::endl;
          }
          else {
            std::cout << "[HiOniaAnalyzer::hltReport] --- L1 prescale was NOT found for TriggerName " << triggerPathName  << " , default L1 prescale value set to 1 " <<  std::endl;
          }
          //compute the total prescale = HLT prescale * L1 prescale
          mapTriggerNameToPrescaleFac_[triggerPathName] = hltPrescale * l1Prescale;
        }
      }
    }
  } else std::cout << "[HiOniaAnalyzer::hltReport] --- TriggerResults NOT valid in current event" << std::endl;

  return;
}

bool 
HiOniaAnalyzer::isAbHadron(int pdgID) {

  if (abs(pdgID) == 511 || abs(pdgID) == 521 || abs(pdgID) == 531 || abs(pdgID) == 5122) return true;
  return false;

}

bool 
HiOniaAnalyzer::isAMixedbHadron(int pdgID, int momPdgID) {

  if ((abs(pdgID) == 511 && abs(momPdgID) == 511 && pdgID*momPdgID < 0) || 
      (abs(pdgID) == 531 && abs(momPdgID) == 531 && pdgID*momPdgID < 0)) 
      return true;
  return false;

}

reco::GenParticleRef   
HiOniaAnalyzer::findMotherRef(reco::GenParticleRef GenParticleMother, int GenParticlePDG) {

  for(int i=0; i<1000; ++i) {
    if (GenParticleMother.isNonnull() && (GenParticleMother->pdgId()==GenParticlePDG) && GenParticleMother->numberOfMothers()>0) {        
      GenParticleMother = GenParticleMother->motherRef();
    } else break;
  }
  return GenParticleMother;

}

std::pair<int, std::pair<float, float> >  
HiOniaAnalyzer::findGenMCInfo(const reco::GenParticle* genJpsi) {

  int momJpsiID = 0;
  float trueLife = -99.;
  float trueLife3D = -99.;

  if (genJpsi->numberOfMothers()>0) {
    TVector3 trueVtx(0.0,0.0,0.0);
    TVector3 trueP(0.0,0.0,0.0);
    TVector3 trueVtxMom(0.0,0.0,0.0);

    trueVtx.SetXYZ(genJpsi->vertex().x(),genJpsi->vertex().y(),genJpsi->vertex().z());
    trueP.SetXYZ(genJpsi->momentum().x(),genJpsi->momentum().y(),genJpsi->momentum().z());

    bool aBhadron = false;
    reco::GenParticleRef Jpsimom = findMotherRef(genJpsi->motherRef(), genJpsi->pdgId());      
    if (Jpsimom.isNull()) {
      std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
      std::pair<int, std::pair<float, float>> result = std::make_pair(momJpsiID, trueLifePair);
      return result;
    } 
    else if (Jpsimom->numberOfMothers()<=0) {
      if (isAbHadron(Jpsimom->pdgId())) {  
        momJpsiID = Jpsimom->pdgId();
        trueVtxMom.SetXYZ(Jpsimom->vertex().x(),Jpsimom->vertex().y(),Jpsimom->vertex().z());
        aBhadron = true;
      }
    } 
    else {
      reco::GenParticleRef Jpsigrandmom = findMotherRef(Jpsimom->motherRef(), Jpsimom->pdgId());   
      if (isAbHadron(Jpsimom->pdgId())) {       
        if (Jpsigrandmom.isNonnull() && isAMixedbHadron(Jpsimom->pdgId(),Jpsigrandmom->pdgId())) {       
          momJpsiID = Jpsigrandmom->pdgId();
          trueVtxMom.SetXYZ(Jpsigrandmom->vertex().x(),Jpsigrandmom->vertex().y(),Jpsigrandmom->vertex().z());
        } 
        else {                  
          momJpsiID = Jpsimom->pdgId();
          trueVtxMom.SetXYZ(Jpsimom->vertex().x(),Jpsimom->vertex().y(),Jpsimom->vertex().z());
        }
        aBhadron = true;
      } 
      else if (Jpsigrandmom.isNonnull() && isAbHadron(Jpsigrandmom->pdgId()))  {  
        if (Jpsigrandmom->numberOfMothers()<=0) {
          momJpsiID = Jpsigrandmom->pdgId();
          trueVtxMom.SetXYZ(Jpsigrandmom->vertex().x(),Jpsigrandmom->vertex().y(),Jpsigrandmom->vertex().z());
        } 
        else { 
          reco::GenParticleRef JpsiGrandgrandmom = findMotherRef(Jpsigrandmom->motherRef(), Jpsigrandmom->pdgId());
          if (JpsiGrandgrandmom.isNonnull() && isAMixedbHadron(Jpsigrandmom->pdgId(),JpsiGrandgrandmom->pdgId())) {
            momJpsiID = JpsiGrandgrandmom->pdgId();
            trueVtxMom.SetXYZ(JpsiGrandgrandmom->vertex().x(),JpsiGrandgrandmom->vertex().y(),JpsiGrandgrandmom->vertex().z());
          } 
          else {
            momJpsiID = Jpsigrandmom->pdgId();
            trueVtxMom.SetXYZ(Jpsigrandmom->vertex().x(),Jpsigrandmom->vertex().y(),Jpsigrandmom->vertex().z());
          }
        }
        aBhadron = true;
      }
    }
    if (!aBhadron) {
      momJpsiID = Jpsimom->pdgId();
      trueVtxMom.SetXYZ(Jpsimom->vertex().x(),Jpsimom->vertex().y(),Jpsimom->vertex().z()); 
    }
    
    TVector3 vdiff = trueVtx - trueVtxMom;
    trueLife = vdiff.Perp()*3.096916/trueP.Perp();
    trueLife3D = vdiff.Mag()*3.096916/trueP.Mag();
  }

  std::pair<float, float> trueLifePair = std::make_pair(trueLife, trueLife3D);
  std::pair<int, std::pair<float, float> > result = std::make_pair(momJpsiID, trueLifePair);
  return result;

}

int HiOniaAnalyzer::muonIDmask(const pat::Muon* muon)
{
   int mask = 0;
   int type;
   for (type=muon::All; type<=muon::RPCMuLoose; type++)
      if (muon::isGoodMuon(*muon,(muon::SelectionType) type))
         mask = mask | (int) pow(2, type);

   return mask;
}

//define this as a plug-in
DEFINE_FWK_MODULE(HiOniaAnalyzer);
