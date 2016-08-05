// File: HLTAnalyzer.cc
// Description:  Example of Analysis driver originally from Jeremy Mans, 
// Date:  13-October-2006

#include <boost/foreach.hpp>

#include "HLTrigger/HLTanalyzers/interface/HLTAnalyzer.h"
#include "HLTMessages.h"

typedef std::pair<const char *, const edm::InputTag *> MissingCollectionInfo;

template <class T>
static inline
bool getCollection(const edm::Event & event, std::vector<MissingCollectionInfo> & missing, edm::Handle<T> & handle, const edm::InputTag & name, const edm::EDGetTokenT<T> token, const char * description) 
{
  event.getByToken(token, handle);
  bool valid = handle.isValid();
  if (not valid) {
    missing.push_back( std::make_pair(description, & name) );
    handle.clear();
    //	std::cout << "not valid "<< description << " " << name << std::endl;
  }
  return valid;
}

// Boiler-plate constructor definition of an analyzer module:
HLTAnalyzer::HLTAnalyzer(edm::ParameterSet const& conf) :
  hlt_analysis_(conf, consumesCollector(), *this) {
    
    // If your module takes parameters, here is where you would define
    // their names and types, and access them to initialize internal
    // variables. Example as follows:
    std::cout << " Beginning HLTAnalyzer Analysis " << std::endl;

    muonFilterCollections_    = conf.getParameter< std::vector< edm::InputTag > > ("muonFilters");

    muon_               = conf.getParameter<edm::InputTag> ("muon");
    mctruth_            = conf.getParameter<edm::InputTag> ("mctruth");
    genEventInfo_       = conf.getParameter<edm::InputTag> ("genEventInfo");
    simhits_            = conf.getParameter<edm::InputTag> ("simhits");
    xSection_           = conf.getUntrackedParameter<double> ("xSection",1.);
    filterEff_          = conf.getUntrackedParameter<double> ("filterEff",1.);
    firstLumi_          = conf.getUntrackedParameter<int> ("firstLumi",0);
    lastLumi_           = conf.getUntrackedParameter<int> ("lastLumi",-1);
       
    // keep this separate from l1extramc_ as needed by FastSim:
    //    This is purposefully done this way to allow FastSim to run with OpenHLT: 
    //    The {FastSim+OpenHLT} package runs on the head of HLTrigger/HLTanalyzers 
    //    where there is purposefully this duplication because FastSim does the 
    //    simulation of muons seperately, and needs the same collection. 
    l1extramu_        = conf.getParameter<std::string>   ("l1extramu");
    m_l1extramu       = edm::InputTag(l1extramu_, "");
    
    // read the L1Extra collection name, and add the instance names as needed
    l1extramc_        = conf.getParameter<std::string>   ("l1extramc");
    m_l1extraemi      = edm::InputTag(l1extramc_, "Isolated");
    m_l1extraemn      = edm::InputTag(l1extramc_, "NonIsolated");
    m_l1extrajetc     = edm::InputTag(l1extramc_, "Central");
    m_l1extrajetf     = edm::InputTag(l1extramc_, "Forward");
    m_l1extrajet      = edm::InputTag("gctInternJetProducer","Internal","reHLT");
    m_l1extrataujet   = edm::InputTag(l1extramc_, "Tau");
    m_l1extramet      = edm::InputTag(l1extramc_, "MET");
    m_l1extramht      = edm::InputTag(l1extramc_, "MHT");

    hltresults_       = conf.getParameter<edm::InputTag> ("hltresults");
    gtReadoutRecord_  = conf.getParameter<edm::InputTag> ("l1GtReadoutRecord");
    
    gctBitCounts_        = edm::InputTag( conf.getParameter<edm::InputTag>("l1GctHFBitCounts").label(), "" );
    gctRingSums_         = edm::InputTag( conf.getParameter<edm::InputTag>("l1GctHFRingSums").label(), "" );
    
    MuCandTag2_          = conf.getParameter<edm::InputTag> ("MuCandTag2");
    MuCandTag3_          = conf.getParameter<edm::InputTag> ("MuCandTag3");

    L3TkTracksFromL2OIStateTag_  = conf.getParameter<edm::InputTag> ("L3TkTracksFromL2OIStateTag");
    L3TkTracksFromL2OIHitTag_    = conf.getParameter<edm::InputTag> ("L3TkTracksFromL2OIHitTag");

    // Reco Vertex collection
    VertexTagHLT_                = conf.getParameter<edm::InputTag> ("PrimaryVertices");  
    VertexTagOffline0_           = conf.getParameter<edm::InputTag> ("OfflinePrimaryVertices0");

    m_file = 0;   // set to null
    errCnt = 0;
    
    // read run parameters with a default value 
    edm::ParameterSet runParameters = conf.getParameter<edm::ParameterSet>("RunParameters");
    _HistName = runParameters.getUntrackedParameter<std::string>("HistogramFile", "test.root");
    _EtaMin   = runParameters.getUntrackedParameter<double>("EtaMin", -5.2);
    _EtaMax   = runParameters.getUntrackedParameter<double>("EtaMax",  5.2);
    

    // Define all consumed products  

    for (unsigned int i=0; i<muonFilterCollections_.size() ; i++) { 
      muonFilterTokens_.push_back( consumes<trigger::TriggerFilterObjectWithRefs>( muonFilterCollections_.at(i) ) );
    }

    BSProducer_ =  edm::InputTag("hltOnlineBeamSpot");
    BSProducerToken_ = consumes<reco::BeamSpot>(BSProducer_);

    muonToken_ = consumes<reco::MuonCollection>(muon_);

    hltresultsToken_ = consumes<edm::TriggerResults>(hltresults_);
    l1extraemiToken_ = consumes<l1extra::L1EmParticleCollection>(m_l1extraemi);
    l1extraemnToken_ = consumes<l1extra::L1EmParticleCollection>(m_l1extraemn);
    l1extramuToken_ = consumes<l1extra::L1MuonParticleCollection>(m_l1extramu);
    
    l1extrajetcToken_ = consumes<l1extra::L1JetParticleCollection>(m_l1extrajetc);
    l1extrajetfToken_ = consumes<l1extra::L1JetParticleCollection>(m_l1extrajetf);
    l1extrajetToken_ = consumes<l1extra::L1JetParticleCollection>(m_l1extrajet);
    l1extrataujetToken_ = consumes<l1extra::L1JetParticleCollection>(m_l1extrataujet);
    l1extrametToken_ = consumes<l1extra::L1EtMissParticleCollection>(m_l1extramet);
    l1extramhtToken_ = consumes<l1extra::L1EtMissParticleCollection>(m_l1extramht);
    gtReadoutRecordToken_ = consumes<L1GlobalTriggerReadoutRecord>(gtReadoutRecord_);
    gctBitCountsToken_ = consumes<L1GctHFBitCountsCollection>(gctBitCounts_);
    gctRingSumsToken_ = consumes<L1GctHFRingEtSumsCollection>(gctRingSums_);

    mctruthToken_ = consumes<reco::CandidateView>(mctruth_);
    simTracksToken_ = consumes<std::vector<SimTrack> >(simhits_);
    simVerticesToken_ = consumes<std::vector<SimVertex> >(simhits_);
    genEventInfoToken_ = consumes<GenEventInfoProduct>(genEventInfo_);

    MuCandTag2Token_ = consumes<reco::RecoChargedCandidateCollection>(MuCandTag2_);
    MuCandTag3Token_ = consumes<reco::RecoChargedCandidateCollection>(MuCandTag3_);

    L3TkTracksFromL2OIStateToken_  = consumes<std::vector<reco::Track>>(L3TkTracksFromL2OIStateTag_);
    L3TkTracksFromL2OIHitToken_    = consumes<std::vector<reco::Track>>(L3TkTracksFromL2OIHitTag_);
        
    VertexHLTToken_ = consumes<reco::VertexCollection>(VertexTagHLT_);
    VertexOffline0Token_ = consumes<reco::VertexCollection>(VertexTagOffline0_);
    
    
    // open the tree file
    m_file = new TFile(_HistName.c_str(), "RECREATE");
    if (m_file)
        m_file->cd();
    
    // Initialize the tree
    HltTree = new TTree("HltTree", "");
    
    treeWeight=xSection_*filterEff_;
    std::cout << "\n Setting HltTree weight to " << treeWeight << " = " << xSection_ << "*" << filterEff_ << " (cross section * gen filter efficiency)\n" << std::endl;
    
    // Setup the different analysis
    muon_analysis_.setup(conf, HltTree);
    hlt_analysis_.setup(conf, HltTree);
    vrt_analysisHLT_.setup(conf, HltTree, "reHLT");
    vrt_analysisOffline0_.setup(conf, HltTree, "Offline0");
    evt_header_.setup(consumesCollector(), HltTree);
}

void HLTAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& c){ 
    
    hlt_analysis_.beginRun(run, c);
}

// Boiler-plate "analyze" method declaration for an analyzer module.
void HLTAnalyzer::analyze(edm::Event const& iEvent, edm::EventSetup const& iSetup) {
    
    // To get information from the event setup, you must request the "Record"
    // which contains it and then extract the object you need
    //edm::ESHandle<CaloGeometry> geometry;
    //iSetup.get<IdealGeometryRecord>().get(geometry);
    
    int iLumi = iEvent.luminosityBlock();
    if (iLumi<firstLumi_) return;
    if (lastLumi_ != -1 && iLumi>lastLumi_) return;
    
    // These declarations create handles to the types of records that you want
    // to retrieve from event "iEvent".

    std::vector< edm::Handle<trigger::TriggerFilterObjectWithRefs> > muonFilterCollections;

    edm::Handle<reco::CandidateView>                  mctruth;
    edm::Handle<GenEventInfoProduct>                  genEventInfo;
    edm::Handle<std::vector<SimTrack> >               simTracks;
    edm::Handle<std::vector<SimVertex> >              simVertices;
    edm::Handle<reco::MuonCollection>                 muon;
    edm::Handle<edm::TriggerResults>                  hltresults;
    edm::Handle<l1extra::L1EmParticleCollection>      l1extemi, l1extemn;
    edm::Handle<l1extra::L1MuonParticleCollection>    l1extmu;
    edm::Handle<l1extra::L1JetParticleCollection>     l1extjetc, l1extjetf, l1extjet, l1exttaujet;
    //edm::Handle<l1extra::L1JetParticleCollection>     l1extjetc, l1extjetf, l1exttaujet;
    edm::Handle<l1extra::L1EtMissParticleCollection>  l1extmet,l1extmht;
    edm::Handle<L1GlobalTriggerReadoutRecord>         l1GtRR;
    edm::Handle< L1GctHFBitCountsCollection >         gctBitCounts ;
    edm::Handle< L1GctHFRingEtSumsCollection >        gctRingSums ;
    
    edm::Handle<reco::RecoChargedCandidateCollection> mucands2, mucands3;
    edm::Handle< std::vector<reco::Track> > L3TkTracksFromL2OIState, L3TkTracksFromL2OIHit;
    
    // Reco vertex collection
    edm::Handle<reco::VertexCollection> recoVertexsHLT;
    edm::Handle<reco::VertexCollection> recoVertexsOffline0;

    edm::ESHandle<MagneticField>                theMagField;
    iSetup.get<IdealMagneticFieldRecord>().get(theMagField);
    
    edm::Handle<reco::BeamSpot> recoBeamSpotHandle;
    
    // extract the collections from the event, check their validity and log which are missing
    std::vector<MissingCollectionInfo> missing;
    
    
    for (unsigned int i=0; i<muonFilterCollections_.size() ; i++) { 
      edm::Handle<trigger::TriggerFilterObjectWithRefs> handle;
      iEvent.getByToken(muonFilterTokens_.at(i), handle);
      muonFilterCollections.push_back(handle);
    }

    //get the BeamSpot
    getCollection( iEvent, missing, recoBeamSpotHandle,   BSProducer_ ,   BSProducerToken_ ,   "Beam Spot handle");
    // gets its position
    reco::BeamSpot::Point BSPosition(0,0,0);
    BSPosition = recoBeamSpotHandle->position();
    getCollection( iEvent, missing, muon,            muon_,              muonToken_,              kMuon );
    getCollection( iEvent, missing, hltresults,      hltresults_,        hltresultsToken_,        kHltresults );
    getCollection( iEvent, missing, l1extemi,        m_l1extraemi,       l1extraemiToken_,        kL1extemi );
    getCollection( iEvent, missing, l1extemn,        m_l1extraemn,       l1extraemnToken_,        kL1extemn );
    getCollection( iEvent, missing, l1extmu,         m_l1extramu,        l1extramuToken_,         kL1extmu );
    getCollection( iEvent, missing, l1extjetc,       m_l1extrajetc,      l1extrajetcToken_,       kL1extjetc );
    getCollection( iEvent, missing, l1extjetf,       m_l1extrajetf,      l1extrajetfToken_,       kL1extjetf );
    getCollection( iEvent, missing, l1extjet,        m_l1extrajet,       l1extrajetToken_,        kL1extjet );
    getCollection( iEvent, missing, l1exttaujet,     m_l1extrataujet,    l1extrataujetToken_,     kL1exttaujet );
    getCollection( iEvent, missing, l1extmet,        m_l1extramet,       l1extrametToken_,        kL1extmet );
    getCollection( iEvent, missing, l1extmht,        m_l1extramht,       l1extramhtToken_,        kL1extmht );
    getCollection( iEvent, missing, l1GtRR,          gtReadoutRecord_,   gtReadoutRecordToken_,   kL1GtRR );
    getCollection( iEvent, missing, gctBitCounts,    gctBitCounts_,      gctBitCountsToken_,      kL1GctBitCounts );
    getCollection( iEvent, missing, gctRingSums,     gctRingSums_,       gctRingSumsToken_,       kL1GctRingSums );
    getCollection( iEvent, missing, mctruth,         mctruth_,           mctruthToken_,           kMctruth );
    getCollection( iEvent, missing, simTracks,       simhits_,           simTracksToken_,         kSimhit );
    getCollection( iEvent, missing, simVertices,     simhits_,           simVerticesToken_,       kSimhit );
    getCollection( iEvent, missing, genEventInfo,    genEventInfo_,      genEventInfoToken_,      kGenEventInfo );
    getCollection( iEvent, missing, mucands2,        MuCandTag2_,        MuCandTag2Token_,        kMucands2 );
    getCollection( iEvent, missing, mucands3,        MuCandTag3_,        MuCandTag3Token_,        kMucands3 );
    getCollection( iEvent, missing, L3TkTracksFromL2OIState, L3TkTracksFromL2OIStateTag_, L3TkTracksFromL2OIStateToken_, "L3TkTracksFromL2OIState" );
    getCollection( iEvent, missing, L3TkTracksFromL2OIHit, L3TkTracksFromL2OIHitTag_, L3TkTracksFromL2OIHitToken_, "L3TkTracksFromL2OIHit" );
    getCollection( iEvent, missing, recoVertexsHLT,           VertexTagHLT_,              VertexHLTToken_,              kRecoVerticesHLT );
    getCollection( iEvent, missing, recoVertexsOffline0,      VertexTagOffline0_,         VertexOffline0Token_,         kRecoVerticesOffline0 );
    
    // print missing collections
    if (not missing.empty() and (errCnt < errMax())) {
        errCnt++;
        std::stringstream out;       
        out <<  "OpenHLT analyser - missing collections (This message is for information only. RECO collections will always be missing when running on RAW, MC collections will always be missing when running on data):";
        BOOST_FOREACH(const MissingCollectionInfo & entry, missing)
        out << "\n\t" << entry.first << ": " << entry.second->encode();
        edm::LogPrint("OpenHLT") << out.str() << std::endl; 
        if (errCnt == errMax())
            edm::LogWarning("OpenHLT") << "Maximum error count reached -- No more messages will be printed.";
    }
    

    muon_analysis_.analyze(
                           iEvent,
                           muon,
                           l1extmu,
                           mucands2,
                           mucands3,
                           L3TkTracksFromL2OIState,
                           L3TkTracksFromL2OIHit,
                           muonFilterCollections,
			   theMagField,
                           recoBeamSpotHandle,
                           HltTree);
    
    hlt_analysis_.analyze(
                          hltresults,
                          l1extemi,
                          l1extemn,
                          l1extmu,
                          l1extjetc,
                          l1extjetf,
			  l1extjet,
                          l1exttaujet,
                          l1extmet,
                          l1extmht,
                          l1GtRR,
                          gctBitCounts,
                          gctRingSums,
                          iSetup,
                          iEvent,
                          HltTree);

    vrt_analysisHLT_.analyze(
			     recoVertexsHLT,
			     HltTree);

    vrt_analysisOffline0_.analyze(
			     recoVertexsOffline0,
			     HltTree);

    evt_header_.analyze(iEvent, HltTree);
    
    
    // std::cout << " Ending Event Analysis" << std::endl;
    // After analysis, fill the variables tree
    if (m_file)
        m_file->cd();
    HltTree->Fill();
}

// "endJob" is an inherited method that you may implement to do post-EOF processing and produce final output.
void HLTAnalyzer::endJob() {
    
    if (m_file)
        m_file->cd();
    
    const edm::ParameterSet &thepset = edm::getProcessParameterSet();   
    TList *list = HltTree->GetUserInfo();   
    list->Add(new TObjString(thepset.dump().c_str()));   
    
    HltTree->SetWeight(treeWeight);
    HltTree->Write();
    delete HltTree;
    HltTree = 0;
    
    if (m_file) {         // if there was a tree file...
        m_file->Write();    // write out the branches
        delete m_file;      // close and delete the file
        m_file = 0;         // set to zero to clean up
    }
    
}
