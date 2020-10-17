#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/IPTools/interface/IPTools.h"

namespace pat {

  class MuonUnpacker : public edm::stream::EDProducer<> {

    public:

      explicit MuonUnpacker(const edm::ParameterSet & iConfig):
          muonSelectors_(iConfig.getParameter<std::vector<std::string> >("muonSelectors")),
          muonToken_(consumes<pat::MuonCollection>(iConfig.getParameter<edm::InputTag>("muons"))),
          trackToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"))),
          primaryVertexToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertices"))),
          beamSpotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpot")))
      {
        for (const auto& sel: muonSelectors_) {
          candidateMuonIDToken_["packedPFCandidate"][sel] = consumes<pat::PackedCandidateRefVector>(edm::InputTag("packedCandidateMuonID", "pfCandidates"+sel));
          candidateMuonIDToken_["lostTrack"][sel] = consumes<pat::PackedCandidateRefVector>(edm::InputTag("packedCandidateMuonID", "lostTracks"+sel));
        }
        produces<pat::MuonCollection>();
      }
      ~MuonUnpacker() override {};

      void produce(edm::Event&, const edm::EventSetup&) override;

      void addMuon(pat::Muon&, const pat::PackedCandidate&, const std::string&,
                   const reco::TrackRef&, const edm::ESHandle<TransientTrackBuilder>&,
                   const reco::Vertex&, const reco::BeamSpot&, std::map<std::string, bool>);

      static void fillDescriptions(edm::ConfigurationDescriptions&);

    private:

      const std::vector<std::string> muonSelectors_;
      const edm::EDGetTokenT<pat::MuonCollection> muonToken_;
      const edm::EDGetTokenT<reco::TrackCollection> trackToken_;
      const edm::EDGetTokenT<reco::VertexCollection> primaryVertexToken_;
      const edm::EDGetTokenT<reco::BeamSpot> beamSpotToken_;
      const edm::EDGetTokenT<edm::TriggerResults> triggerResultToken_;
      std::map<std::string, std::map<std::string, edm::EDGetTokenT<pat::PackedCandidateRefVector> > > candidateMuonIDToken_;

  };

}

void pat::MuonUnpacker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // extract candidate map
  std::map<std::string, std::map<pat::PackedCandidateRef, std::map<std::string, bool> > > candidateMuonIDs;
  for (const auto& n : candidateMuonIDToken_) {
    for (const auto& s: n.second) {
      const auto& coll = iEvent.getHandle(s.second);
      if (coll.isValid()) {
        for (const auto& c : *coll) { 
          candidateMuonIDs[n.first][c][s.first] = true;
        }
      }
    }
  }

  // extract candidate track map
  typedef std::tuple<float, float, float, char, unsigned short> TUPLE;
  std::map<TUPLE, reco::TrackRef> candTrackMap;
  const auto& tracks = iEvent.getHandle(trackToken_);
  for (size_t i=0; i<tracks->size(); i++) {
    const auto& trk = reco::TrackRef(tracks, i);
    const auto& tuple = std::make_tuple(trk->pt(), trk->eta(), trk->phi(), trk->charge(), trk->numberOfValidHits());
    candTrackMap[tuple] = trk;
  }

  // extract primary vertex and beamspot
  const auto& primaryVertex = iEvent.get(primaryVertexToken_)[0];
  const auto& beamSpot = iEvent.get(beamSpotToken_);

  // extract the transient track builder
  edm::ESHandle<TransientTrackBuilder> trackBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", trackBuilder);

  // extract muons from packedCandidate collections
  pat::MuonCollection candMuons;
  const auto& muons = iEvent.get(muonToken_);
  for (const auto& n : candidateMuonIDs) {
    for (const auto& c : n.second) {
      const auto& cand = c.first;
      const auto& selMap = c.second;

      // extract candidate track
      if (cand->charge()==0 || !cand->hasTrackDetails()) continue;
      const auto& trk = cand->pseudoTrack();
      const auto& tuple = std::make_tuple(trk.pt(), trk.eta(), trk.phi(), trk.charge(), trk.numberOfValidHits());
      const auto& track = candTrackMap.at(tuple);

      // check if candidate is in muon collection
      bool isIncluded = false;
      for (const auto& muon : muons) {
        const auto& mtrk = muon.innerTrack();
        if (mtrk.isNull() || !mtrk->quality(reco::TrackBase::qualityByName("highPurity"))) continue;
        if (mtrk->charge()==track->charge() && mtrk->numberOfValidHits()==track->numberOfValidHits() &&
            std::abs(mtrk->eta()-track->eta())<1E-3 && std::abs(mtrk->phi()-track->phi())<1E-3 && std::abs((mtrk->pt()-track->pt())/mtrk->pt())<1E-2) {
          isIncluded = true;
          break;
        }
      }
      if (isIncluded) continue;

      // create and add the muon object
      pat::Muon muon;
      addMuon(muon, *cand, n.first, track, trackBuilder, primaryVertex, beamSpot, selMap);
      candMuons.push_back(muon);
    }
  }

  // create output and add extra muons from packedCandidate collections
  auto output = std::make_unique<pat::MuonCollection>(muons);
  for (const auto& candMuon : candMuons) { output->push_back(candMuon); }
  
  // clear trigger data from MiniAOD
  for (auto& muon : *output) {
    const_cast<TriggerObjectStandAloneCollection*>(&muon.triggerObjectMatches())->clear();
  }

  iEvent.put(std::move(output));
}

void pat::MuonUnpacker::addMuon(pat::Muon& muon, const pat::PackedCandidate& cand, const std::string& lbl,
                                const reco::TrackRef& track, const edm::ESHandle<TransientTrackBuilder>& trackBuilder,
                                const reco::Vertex& primaryVertex, const reco::BeamSpot& beamSpot,
                                std::map<std::string, bool> selMap) {
  // add basic information
  muon.setP4(cand.p4());
  muon.setCharge(cand.charge());
  muon.setVertex(cand.vertex());
  muon.setPdgId(-13 * cand.charge());

  // add track information
  muon.setInnerTrack(track);
  muon.setMuonTrack(reco::Muon::InnerTrack, track);
  muon.setTrack(track);
  muon.setBestTrack(reco::Muon::InnerTrack);
  muon.embedTrack();
  muon.embedMuonBestTrack();
  muon.setNumberOfValidHits(track->numberOfValidHits());
  const auto& muonTrack = *muon.track();

  // add vertex information
  auto tt = trackBuilder->build(muonTrack);
  auto result = IPTools::signedTransverseImpactParameter(tt, GlobalVector(muonTrack.px(), muonTrack.py(), muonTrack.pz()), primaryVertex);
  muon.setDB(result.second.value(), result.second.error(), pat::Muon::PV2D);
  result = IPTools::signedImpactParameter3D(tt, GlobalVector(muonTrack.px(), muonTrack.py(), muonTrack.pz()), primaryVertex);
  muon.setDB(result.second.value(), result.second.error(), pat::Muon::PV3D);
  reco::Vertex beamSpotVertex(beamSpot.position(), beamSpot.rotatedCovariance3D());
  result = IPTools::signedTransverseImpactParameter(tt, GlobalVector(muonTrack.px(), muonTrack.py(), muonTrack.pz()), beamSpotVertex);
  muon.setDB(result.second.value(), result.second.error(), pat::Muon::BS2D);
  result = IPTools::signedImpactParameter3D(tt, GlobalVector(muonTrack.px(), muonTrack.py(), muonTrack.pz()), beamSpotVertex);
  muon.setDB(result.second.value(), result.second.error(), pat::Muon::BS3D);
  muon.setDB(muonTrack.dz(primaryVertex.position()), std::hypot(muonTrack.dzError(), primaryVertex.zError()), pat::Muon::PVDZ);

  // determine muon selectors
  bool isTrackerMuon = selMap["AllTrackerMuons"] || selMap["TMOneStationTight"];
  bool isGlobalMuon = selMap["AllGlobalMuons"] || cand.isGlobalMuon();
  bool isStandAloneMuon = selMap["AllStandAloneMuons"] || cand.isStandAloneMuon();
  bool isPFMuon = (lbl=="packedPFCandidate" && std::abs(cand.pdgId())==13);
  bool isHPMuon = muonTrack.quality(reco::Track::highPurity);
  bool isLooseMuon = isPFMuon || isGlobalMuon || isTrackerMuon;
  bool isTMOneStationTight = selMap["TMOneStationTight"];
  bool isSoftMuon = isTMOneStationTight && isHPMuon &&
                    muonTrack.hitPattern().trackerLayersWithMeasurement()>5 && muonTrack.hitPattern().pixelLayersWithMeasurement()>0 &&
                    fabs(muonTrack.dxy(primaryVertex.position()))<0.3 && fabs(muonTrack.dz(primaryVertex.position()))<20.;

  // add muon selectors
  unsigned int type = 0;
  unsigned int selectors = 0;
  std::vector<bool> selVec(muon::TriggerIdLoose+1, false);
  if (isLooseMuon) {
    selectors |= reco::Muon::CutBasedIdLoose;
  }
  if (isSoftMuon) {
    selectors |= reco::Muon::SoftCutBasedId;
  }
  selVec[muon::All] = true;
  if (isTrackerMuon) {
    type |= reco::Muon::TrackerMuon;
    selVec[muon::AllTrackerMuons] = true;
  }
  if (isStandAloneMuon) {
    type |= reco::Muon::StandAloneMuon;
    selVec[muon::AllStandAloneMuons] = true;
  }
  if (isGlobalMuon) {
    //type |= reco::Muon::GlobalMuon;
    selVec[muon::AllGlobalMuons] = true;
  }
  if (isPFMuon) {
    type |= reco::Muon::PFMuon;
    muon.setPFP4(cand.p4());
  }
  if (isTMOneStationTight) {
    selVec[muon::TMOneStationTight] = true;
  }
  muon.setType(type);
  muon.setSelectors(selectors);
  for (int i=muon::All; i<=muon::TriggerIdLoose; i++) {
    muon.addUserInt(Form("muonID_%d", i), selVec[i]);
  }

  // add candidate information
  unsigned int candType = 0;
  if (lbl=="packedPFCandidate") {
    candType = 1;
  }
  else if (lbl=="lostTrack") {
    candType = 2;
  }
  muon.addUserInt("candType", candType);
}
  

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void pat::MuonUnpacker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("muons", edm::InputTag("slimmedMuons"))->setComment("muon input collection");
  desc.add<edm::InputTag>("tracks", edm::InputTag("unpackedTracksAndVertices"))->setComment("track input collection");
  desc.add<edm::InputTag>("primaryVertices", edm::InputTag("unpackedTracksAndVertices"))->setComment("primary vertex input collection");
  desc.add<edm::InputTag>("beamSpot", edm::InputTag("offlineBeamSpot"))->setComment("beam spot collection");
  desc.add<std::vector<std::string> >("muonSelectors", {"AllTrackerMuons", "TMOneStationTight"})->setComment("muon selectors");
  descriptions.add("unpackedMuons", desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
using namespace pat;
DEFINE_FWK_MODULE(MuonUnpacker);
