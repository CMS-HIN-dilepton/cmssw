#ifndef HiSkim_HiOnia2MuMu_HiOnia2MuMuPAT_h
#define HiSkim_HiOnia2MuMu_HiOnia2MuMuPAT_h


// system include files
#include <memory>

// FW include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CommonTools/Utils/interface/PtComparator.h"

// DataFormat includes

#include "DataFormats/Provenance/interface/Provenance.h"
#include <DataFormats/PatCandidates/interface/CompositeCandidate.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include <CommonTools/UtilAlgos/interface/StringCutObjectSelector.h>
#include "RecoVertex/VertexTools/interface/InvariantMassFromVertex.h"


template<typename T>
struct GreaterByVProb {
  typedef T first_argument_type;
  typedef T second_argument_type;
  bool operator()( const T & t1, const T & t2 ) const {
    return t1.userFloat("vProb") > t2.userFloat("vProb");
  }
};


//
// class declaration
//

class HiOnia2MuMuPAT : public edm::EDProducer {
  public:
    explicit HiOnia2MuMuPAT(const edm::ParameterSet&);
    ~HiOnia2MuMuPAT();

  private:
    virtual void beginJob() ;
    virtual void produce(edm::Event&, const edm::EventSetup&);
    virtual void endJob() ;
    bool isAbHadron(int pdgID);
    bool isAMixedbHadron(int pdgID, int momPdgID);
    reco::GenParticleRef findMotherRef(reco::GenParticleRef GenParticle, int GenParticlePDG);
    std::pair<int, std::pair<float, float> > findJpsiMCInfo(reco::GenParticleRef genJpsi);

  // ----------member data ---------------------------
  private:
    edm::EDGetTokenT< edm::View<pat::Muon> >        muonsToken_;         
    edm::EDGetTokenT<reco::BeamSpot>                thebeamspotToken_;
    edm::EDGetTokenT<reco::VertexCollection>        thePVsToken_;
    edm::EDGetTokenT<reco::TrackCollection>         recoTracksToken_;
    edm::EDGetTokenT<reco::GenParticleCollection>   theGenParticlesToken_;
    StringCutObjectSelector<pat::Muon> higherPuritySelection_;
    StringCutObjectSelector<pat::Muon> lowerPuritySelection_; 
    StringCutObjectSelector<reco::Candidate, true> dimuonSelection_;
    bool addCommonVertex_, addMuonlessPrimaryVertex_;
    bool resolveAmbiguity_;
    bool addMCTruth_;
    GreaterByPt<pat::CompositeCandidate> pTComparator_;
    GreaterByVProb<pat::CompositeCandidate> vPComparator_;

    InvariantMassFromVertex massCalculator;
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

#endif
