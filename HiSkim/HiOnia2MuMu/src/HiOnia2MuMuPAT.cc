#include "HiSkim/HiOnia2MuMu/interface/HiOnia2MuMuPAT.h"

//Headers for the data items
#include <DataFormats/TrackReco/interface/TrackFwd.h>
#include <DataFormats/TrackReco/interface/Track.h>
#include <DataFormats/MuonReco/interface/MuonFwd.h>
#include <DataFormats/MuonReco/interface/Muon.h>
#include <DataFormats/Common/interface/View.h>
#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <DataFormats/PatCandidates/interface/Muon.h>
#include <DataFormats/VertexReco/interface/VertexFwd.h>

//Headers for services and tools
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/PrimaryVertexProducer/interface/PrimaryVertexProducer.h"
#include "RecoVertex/VertexPrimitives/interface/TransientVertex.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"     
#include "TMath.h"
#include "Math/VectorUtil.h"
#include "TVector3.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "TrackingTools/PatternTools/interface/TwoTrackMinimumDistance.h"
#include "TrackingTools/IPTools/interface/IPTools.h"
#include "TrackingTools/PatternTools/interface/ClosestApproachInRPhi.h"

HiOnia2MuMuPAT::HiOnia2MuMuPAT(const edm::ParameterSet& iConfig):
  muonsToken_(consumes< edm::View<pat::Muon> >(iConfig.getParameter<edm::InputTag>("muons"))),
  thebeamspotToken_(consumes<reco::BeamSpot>(iConfig.getParameter<edm::InputTag>("beamSpotTag"))),
  thePVsToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertexTag"))),
  recoTracksToken_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("srcTracks"))),
  theGenParticlesToken_(consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticles"))),
  higherPuritySelection_(iConfig.getParameter<std::string>("higherPuritySelection")),
  lowerPuritySelection_(iConfig.getParameter<std::string>("lowerPuritySelection")),
  dimuonSelection_(iConfig.existsAs<std::string>("dimuonSelection") ? iConfig.getParameter<std::string>("dimuonSelection") : ""),
  addCommonVertex_(iConfig.getParameter<bool>("addCommonVertex")),
  addMuonlessPrimaryVertex_(iConfig.getParameter<bool>("addMuonlessPrimaryVertex")),
  resolveAmbiguity_(iConfig.getParameter<bool>("resolvePileUpAmbiguity")),
  addMCTruth_(iConfig.getParameter<bool>("addMCTruth"))
{  
  produces<pat::CompositeCandidateCollection>();  
}


HiOnia2MuMuPAT::~HiOnia2MuMuPAT()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HiOnia2MuMuPAT::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{  
  using namespace edm;
  using namespace std;
  using namespace reco;
  typedef Candidate::LorentzVector LorentzVector;

  vector<double> muMasses;
  muMasses.push_back( 0.1056583715 );
  muMasses.push_back( 0.1056583715 );

  std::auto_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);
  
  Vertex thePrimaryV;
  Vertex theBeamSpotV; 

  ESHandle<MagneticField> magneticField;
  iSetup.get<IdealMagneticFieldRecord>().get(magneticField);

  // get the stored reco BS, and copy its position in a Vertex object (theBeamSpotV)
  Handle<BeamSpot> theBeamSpot;
  iEvent.getByToken(thebeamspotToken_,theBeamSpot);
  BeamSpot bs = *theBeamSpot;
  theBeamSpotV = Vertex(bs.position(), bs.covariance3D());

  Handle<VertexCollection> priVtxs;
  iEvent.getByToken(thePVsToken_, priVtxs);
  if ( priVtxs->begin() != priVtxs->end() ) {
    thePrimaryV = Vertex(*(priVtxs->begin()));
  }
  else {
    thePrimaryV = Vertex(bs.position(), bs.covariance3D());
  }
 
  // // ---------------------------
  Handle< View<pat::Muon> > muons;
  iEvent.getByToken(muonsToken_ , muons);

  edm::ESHandle<TransientTrackBuilder> theTTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theTTBuilder);
  KalmanVertexFitter vtxFitter(true);
  TrackCollection muonLess; // track collection related to PV, minus the 2 minus (if muonLessPV option is activated)

  Handle<reco::TrackCollection> collTracks;
  iEvent.getByToken(recoTracksToken_,collTracks);
  int Ntrk = -1; 
  if ( collTracks.isValid() ) {
    Ntrk = 0;
    for(std::vector<reco::Track>::const_iterator it=collTracks->begin(); it!=collTracks->end(); ++it) {
      const reco::Track* track = &(*it);        
      if ( track->qualityByName("highPurity") ) { Ntrk++; }
    }
  }

  // JPsi candidates only from muons
  for(View<pat::Muon>::const_iterator it = muons->begin(), itend = muons->end(); it != itend; ++it){
    // both must pass low quality
    if(!lowerPuritySelection_(*it)) continue; 
    for(View<pat::Muon>::const_iterator it2 = it+1; it2 != itend;++it2){
      // both must pass low quality
      if(!lowerPuritySelection_(*it2)) continue; 
      // one must pass tight quality
      if (!(higherPuritySelection_(*it) || higherPuritySelection_(*it2))) continue;

      pat::CompositeCandidate myCand;
      // ---- no explicit order defined ----
      myCand.addDaughter(*it, "muon1");
      myCand.addDaughter(*it2,"muon2"); 

      // ---- define and set candidate's 4momentum  ----  
      LorentzVector jpsi = it->p4() + it2->p4();
      myCand.setP4(jpsi);
      myCand.setCharge(it->charge()+it2->charge());

      std::map< std::string, int > userInt;
      std::map< std::string, float > userFloat;
      std::map< std::string, reco::Vertex > userVertex;

      // ---- apply the dimuon cut ----
      if(!dimuonSelection_(myCand)) continue;

      // ---- fit vertex using Tracker tracks (if they have tracks) ----
      if (it->track().isNonnull() && it2->track().isNonnull()) {
        
        //build the dimuon secondary vertex      

        vector<TransientTrack> t_tks;
        t_tks.push_back(theTTBuilder->build(*it->track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
        t_tks.push_back(theTTBuilder->build(*it2->track())); // otherwise the vertex will have transient refs inside.
        TransientVertex myVertex = vtxFitter.vertex(t_tks);

        CachingVertex<5> VtxForInvMass = vtxFitter.vertex( t_tks );
        Measurement1D MassWErr = massCalculator.invariantMass( VtxForInvMass, muMasses );
        userFloat["MassErr"] = MassWErr.error();

        if (myVertex.isValid()) {
          float vChi2 = myVertex.totalChiSquared();
          float vNDF  = myVertex.degreesOfFreedom();
          float vProb(TMath::Prob(vChi2,(int)vNDF));
          
          userFloat["vNChi2"] = (vChi2/vNDF);
          userFloat["vProb"] = vProb;

          TVector3 vtx, vtx3D;
          TVector3 pvtx, pvtx3D;
          VertexDistanceXY vdistXY;
          VertexDistance3D vdistXYZ;

          vtx.SetXYZ(myVertex.position().x(),myVertex.position().y(),0);
          TVector3 pperp(jpsi.px(), jpsi.py(), 0);
          AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

          vtx3D.SetXYZ(myVertex.position().x(),myVertex.position().y(),myVertex.position().z());
          TVector3 pxyz(jpsi.px(), jpsi.py(), jpsi.pz());
          AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

          if (resolveAmbiguity_) {
            float minDz = 999999.;
            
            TwoTrackMinimumDistance ttmd;
            bool status = ttmd.calculate( GlobalTrajectoryParameters(
                                                                     GlobalPoint(myVertex.position().x(), myVertex.position().y(), myVertex.position().z()),
                                                                     GlobalVector(myCand.px(),myCand.py(),myCand.pz()),TrackCharge(0),&(*magneticField)),
                                          GlobalTrajectoryParameters(
                                                                     GlobalPoint(bs.position().x(), bs.position().y(), bs.position().z()),
                                                                     GlobalVector(bs.dxdz(), bs.dydz(), 1.),TrackCharge(0),&(*magneticField)));
            float extrapZ=-9E20;
            if (status) extrapZ=ttmd.points().first.z();
            
            for(VertexCollection::const_iterator itv = priVtxs->begin(), itvend = priVtxs->end(); itv != itvend; ++itv) {
              // only consider good vertices
              if ( itv->isFake() || fabs(itv->position().z()) > 25 || itv->position().Rho() > 2 || itv->tracksSize() < 2) continue;
              float deltaZ = fabs(extrapZ - itv->position().z()) ;
              if ( deltaZ < minDz ) {
                minDz = deltaZ;    
                thePrimaryV = Vertex(*itv);
              }
            }
          }//if resolve ambiguity

          Vertex theOriginalPV = thePrimaryV;
          muonLess.clear();
          muonLess.reserve(thePrimaryV.tracksSize());
          if( addMuonlessPrimaryVertex_ && thePrimaryV.tracksSize()>2) {
            // Primary vertex matched to the dimuon, now refit it removing the two muons
            //edm::LogWarning("HiOnia2MuMuPAT_addMuonlessPrimaryVertex") << "If muonLessPV is turned on, ctau is calculated with muonLessPV only.\n" ;

            // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
            const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it->originalObject());
            const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2->originalObject());
            if( thePrimaryV.hasRefittedTracks() ) {
              // Need to go back to the original tracks before taking the key
              std::vector<reco::Track>::const_iterator itRefittedTrack   = thePrimaryV.refittedTracks().begin();
              std::vector<reco::Track>::const_iterator refittedTracksEnd = thePrimaryV.refittedTracks().end();
              for( ; itRefittedTrack != refittedTracksEnd; ++itRefittedTrack ) {
                if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu1->track().key() ) continue;
                if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu2->track().key() ) continue;

                const reco::Track & recoTrack = *(thePrimaryV.originalTrack(*itRefittedTrack));
                muonLess.push_back(*(thePrimaryV.originalTrack(*itRefittedTrack)));
              }
            }// PV has refitted tracks
            else 
            {
              std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
              for( ; itPVtrack != thePrimaryV.tracks_end(); ++itPVtrack ) if (itPVtrack->isNonnull()) {
                if( itPVtrack->key() == rmu1->track().key() ) continue;
                if( itPVtrack->key() == rmu2->track().key() ) continue;
                muonLess.push_back(**itPVtrack);
              }
            }// take all tracks associated with the vtx

            if (muonLess.size()>1 && muonLess.size() < thePrimaryV.tracksSize()) {
              // find the new vertex, from which the 2 munos were removed
              // need the transient tracks corresponding to the new track collection
              std::vector<reco::TransientTrack> t_tks; 
              t_tks.reserve(muonLess.size());

              for (reco::TrackCollection::const_iterator it = muonLess.begin(), ed = muonLess.end(); it != ed; ++it) {
                t_tks.push_back((*theTTBuilder).build(*it));
                t_tks.back().setBeamSpot(bs);
              }
              AdaptiveVertexFitter* theFitter=new AdaptiveVertexFitter();
              TransientVertex pvs = theFitter->vertex(t_tks, bs);  // if you want the beam constraint

              if (pvs.isValid()) {
                reco::Vertex muonLessPV = Vertex(pvs);
                thePrimaryV = muonLessPV;
              } else {
                edm::LogWarning("HiOnia2MuMuPAT_FailingToRefitMuonLessVtx") << 
                "TransientVertex re-fitted is not valid!! You got still the 'old vertex'" << "\n";
              }
            } else {
              if ( muonLess.size()==thePrimaryV.tracksSize() ){
                edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
                  "Still have the original PV: the refit was not done 'cose it is already muonless" << "\n";
              } else if ( muonLess.size()<=1 ){
                edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
                  "Still have the original PV: the refit was not done 'cose there are not enough tracks to do the refit without the muon tracks" << "\n";
              } else {
                edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
                  "Still have the original PV: Something weird just happend, muonLess.size()=" << muonLess.size() << " and thePrimaryV.tracksSize()=" << thePrimaryV.tracksSize() << " ." << "\n";
              }
            }
          }// refit vtx without the muon tracks

          // count the number of high Purity tracks with pT > 900 MeV attached to the chosen vertex      
          // this makes sense only in case of pp reconstruction
          double vertexWeight = -1., sumPTPV = -1.;
          int countTksOfPV = -1;
         
          EDConsumerBase::Labels thePVsLabel;
          EDConsumerBase::labelsForToken(thePVsToken_, thePVsLabel);
          if(thePVsLabel.module==(std::string)("offlinePrimaryVertices")) {
            const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it->originalObject());
            const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2->originalObject());
            try {
              for(reco::Vertex::trackRef_iterator itVtx = theOriginalPV.tracks_begin(); itVtx != theOriginalPV.tracks_end(); itVtx++) if(itVtx->isNonnull()) {

                const reco::Track& track = **itVtx;
                if(!track.quality(reco::TrackBase::highPurity)) continue;
                if(track.pt() < 0.5) continue; //reject all rejects from counting if less than 900 MeV

                TransientTrack tt = theTTBuilder->build(track);
                pair<bool,Measurement1D> tkPVdist = IPTools::absoluteImpactParameter3D(tt,theOriginalPV);

                if (!tkPVdist.first) continue;
                if (tkPVdist.second.significance()>3) continue;
                if (track.ptError()/track.pt()>0.1) continue;

                // do not count the two muons
                if (rmu1 != 0 && rmu1->innerTrack().key() == itVtx->key()) continue;
                if (rmu2 != 0 && rmu2->innerTrack().key() == itVtx->key()) continue;

                vertexWeight += theOriginalPV.trackWeight(*itVtx);
                if(theOriginalPV.trackWeight(*itVtx) > 0.5){
                  countTksOfPV++;
                  sumPTPV += track.pt();
                }
              }
            } catch (std::exception & err) {std::cout << " Counting tracks from PV, fails! " << std::endl; return ; }
          }
          userInt["countTksOfPV"] = countTksOfPV;
          userFloat["vertexWeight"] = (float) vertexWeight;
          userFloat["sumPTPV"] = (float) sumPTPV;

          ///DCA
          TrajectoryStateClosestToPoint mu1TS = t_tks[0].impactPointTSCP();
          TrajectoryStateClosestToPoint mu2TS = t_tks[1].impactPointTSCP();
          float dca = 1E20;
          if (mu1TS.isValid() && mu2TS.isValid()) {
            ClosestApproachInRPhi cApp;
            cApp.calculate(mu1TS.theState(), mu2TS.theState());
            if (cApp.status() ) dca = cApp.distance();
          }
          userFloat["DCA"] = dca;
          ///end DCA
         
          if (addMuonlessPrimaryVertex_) {
            userVertex["muonlessPV"] = thePrimaryV;
            userVertex["PVwithmuons"] = theOriginalPV;
          } else {
            userVertex["PVwithmuons"] = thePrimaryV;
          }
          
          // lifetime using PV
          pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
          TVector3 vdiff = vtx - pvtx;
          double cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
          Measurement1D distXY = vdistXY.distance(Vertex(myVertex), thePrimaryV);
          double ctauPV = distXY.value()*cosAlpha*3.096916/pperp.Perp();
          GlobalError v1e = (Vertex(myVertex)).error();
          GlobalError v2e = thePrimaryV.error();
          AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
          double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*3.096916/(pperp.Perp2());

          userFloat["ppdlPV"] = ctauPV;
          userFloat["ppdlErrPV"] = ctauErrPV;
          userFloat["cosAlpha"] = cosAlpha;

          pvtx3D.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),thePrimaryV.position().z());
          TVector3 vdiff3D = vtx3D - pvtx3D;
          double cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
          Measurement1D distXYZ = vdistXYZ.distance(Vertex(myVertex), thePrimaryV);
          double ctauPV3D = distXYZ.value()*cosAlpha3D*3.096916/pxyz.Mag();
          double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYe))*3.096916/(pxyz.Mag2());
          
          userFloat["ppdlPV3D"] = ctauPV3D;
          userFloat["ppdlErrPV3D"] = ctauErrPV3D;
          userFloat["cosAlpha3D"] = cosAlpha3D;

          // lifetime using BS
          pvtx.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),0);
          vdiff = vtx - pvtx;
          cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
          distXY = vdistXY.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS = distXY.value()*cosAlpha*3.096916/pperp.Perp();
          GlobalError v1eB = (Vertex(myVertex)).error();
          GlobalError v2eB = theBeamSpotV.error();
          AlgebraicSymMatrix33 vXYeB = v1eB.matrix()+ v2eB.matrix();
          double ctauErrBS = sqrt(ROOT::Math::Similarity(vpperp,vXYeB))*3.096916/(pperp.Perp2());
          
          userFloat["ppdlBS"] = ctauBS;
          userFloat["ppdlErrBS"] = ctauErrBS;
          pvtx3D.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),theBeamSpotV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theBeamSpotV);
          double ctauBS3D = distXYZ.value()*cosAlpha3D*3.096916/pxyz.Mag();
          double ctauErrBS3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYeB))*3.096916/(pxyz.Mag2());

          userFloat["ppdlBS3D"] = ctauBS3D;
          userFloat["ppdlErrBS3D"] = ctauErrBS3D;
          
          // lifetime using Original PV
          pvtx.SetXYZ(theOriginalPV.position().x(),theOriginalPV.position().y(),0);
          vdiff = vtx - pvtx;
          double cosAlphaOrigPV = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
          distXY = vdistXY.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV = distXY.value()*cosAlphaOrigPV*3.096916/pperp.Perp();
          GlobalError v1eOrigPV = (Vertex(myVertex)).error();
          GlobalError v2eOrigPV = theOriginalPV.error();
          AlgebraicSymMatrix33 vXYeOrigPV = v1eOrigPV.matrix()+ v2eOrigPV.matrix();
          double ctauErrOrigPV = sqrt(ROOT::Math::Similarity(vpperp,vXYeOrigPV))*3.096916/(pperp.Perp2());

          userFloat["ppdlOrigPV"] = ctauOrigPV;
          userFloat["ppdlErrOrigPV"] = ctauErrOrigPV;

          pvtx3D.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), theOriginalPV.position().z());
          vdiff3D = vtx3D - pvtx3D;
          double cosAlphaOrigPV3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
          distXYZ = vdistXYZ.distance(Vertex(myVertex), theOriginalPV);
          double ctauOrigPV3D = distXYZ.value()*cosAlphaOrigPV3D*3.096916/pxyz.Mag();
          double ctauErrOrigPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYeOrigPV))*3.096916/(pxyz.Mag2());
          
          userFloat["ppdlOrigPV3D"] = ctauOrigPV3D;
          userFloat["ppdlErrOrigPV3D"] = ctauErrOrigPV3D;
          
          if (addCommonVertex_) {
            userVertex["commonVertex"] = Vertex(myVertex);
          }
        } else {
          userFloat["vNChi2"] = -1;
          userFloat["vProb"] = -1;
          userFloat["vertexWeight"] = -100;
          userFloat["sumPTPV"] = -100;
          userFloat["DCA"] = -100;
          userFloat["ppdlPV"] = -100;
          userFloat["ppdlErrPV"] = -100;
          userFloat["cosAlpha"] = -100;
          userFloat["ppdlBS"] = -100;
          userFloat["ppdlErrBS"] = -100;
          userFloat["ppdlOrigPV"] = -100;
          userFloat["ppdlErrOrigPV"] = -100;
          userFloat["ppdlPV3D"] = -100;
          userFloat["ppdlErrPV3D"] = -100;
          userFloat["cosAlpha3D"] = -100;
          userFloat["ppdlBS3D"] = -100;
          userFloat["ppdlErrBS3D"] = -100;
          userFloat["ppdlOrigPV3D"] = -100;
          userFloat["ppdlErrOrigPV3D"] = -100;

          userInt["countTksOfPV"] = -1;

          if (addCommonVertex_) {
            userVertex["commonVertex"] = Vertex();
          }
          if (addMuonlessPrimaryVertex_) {
            userVertex["muonlessPV"] = Vertex();
            userVertex["PVwithmuons"] = Vertex();
          } else {
            userVertex["PVwithmuons"] = Vertex();
          }
        }
      } else {
        userFloat["vNChi2"] = -1;
        userFloat["vProb"] = -1;
        userFloat["vertexWeight"] = -100;
        userFloat["sumPTPV"] = -100;
        userFloat["DCA"] = -100;
        userFloat["ppdlPV"] = -100;
        userFloat["ppdlErrPV"] = -100;
        userFloat["cosAlpha"] = -100;
        userFloat["ppdlBS"] = -100;
        userFloat["ppdlErrBS"] = -100;
        userFloat["ppdlOrigPV"] = -100;
        userFloat["ppdlErrOrigPV"] = -100;
        userFloat["ppdlPV3D"] = -100;
        userFloat["ppdlErrPV3D"] = -100;
        userFloat["cosAlpha3D"] = -100;
        userFloat["ppdlBS3D"] = -100;
        userFloat["ppdlErrBS3D"] = -100;
        userFloat["ppdlOrigPV3D"] = -100;
        userFloat["ppdlErrOrigPV3D"] = -100;
        
        userInt["countTksOfPV"] = -1;
        
        if (addCommonVertex_) {
          userVertex["commonVertex"] = Vertex();
        }
        if (addMuonlessPrimaryVertex_) {
          userVertex["muonlessPV"] = Vertex();
          userVertex["PVwithmuons"] = Vertex();
        } else {
          userVertex["PVwithmuons"] = Vertex();
        }
      }
      
      // ---- MC Truth, if enabled ----
      if (addMCTruth_) {
        reco::GenParticleRef genMu1 = it->genParticleRef();
        reco::GenParticleRef genMu2 = it2->genParticleRef();
        if (genMu1.isNonnull() && genMu2.isNonnull()) {
          if (genMu1->numberOfMothers()>0 && genMu2->numberOfMothers()>0){
            if ( !(abs(genMu1->pdgId())==13) || !(abs(genMu2->pdgId())==13) ) { 
              std::cout << "Warning: Generated particles are not muons, pdgID1: " << genMu1->pdgId() << " and pdgID2: " <<  genMu1->pdgId() << std::endl;
            }
            reco::GenParticleRef mom1 = findMotherRef(genMu1->motherRef(), genMu1->pdgId());
            reco::GenParticleRef mom2 = findMotherRef(genMu2->motherRef(), genMu2->pdgId());
            if (mom1.isNonnull() && (mom1 == mom2)) {
              myCand.setGenParticleRef(mom1); // set
              myCand.embedGenParticle();      // and embed
              std::pair<int, std::pair<float, float> > MCinfo = findJpsiMCInfo(mom1);
              userInt["momPDGId"] = MCinfo.first;
              userFloat["ppdlTrue"] = MCinfo.second.first;
              userFloat["ppdlTrue3D"] = MCinfo.second.second;
            } else {
              userInt["momPDGId"] =  0;
              userFloat["ppdlTrue"] = -99.;
              userFloat["ppdlTrue3D"] = -99.;
            }
          } else {
            Handle<GenParticleCollection>theGenParticles;
            iEvent.getByToken(theGenParticlesToken_, theGenParticles);
            if (theGenParticles.isValid()){
              for(size_t iGenParticle=0; iGenParticle<theGenParticles->size();++iGenParticle) {
                const Candidate & genCand = (*theGenParticles)[iGenParticle];
                if (genCand.pdgId()==443 || genCand.pdgId()==100443 || 
                    genCand.pdgId()==553 || genCand.pdgId()==100553 || genCand.pdgId()==200553) {
                  reco::GenParticleRef mom1(theGenParticles,iGenParticle);
                  myCand.setGenParticleRef(mom1);
                  myCand.embedGenParticle();
                  std::pair<int, std::pair<float, float> > MCinfo = findJpsiMCInfo(mom1);
                  userInt["momPDGId"] = MCinfo.first;
                  userFloat["ppdlTrue"] = MCinfo.second.first;
                  userFloat["ppdlTrue3D"] = MCinfo.second.second;
                }
              }
            } else {
              userInt["momPDGId"] =  0;
              userFloat["ppdlTrue"] = -99.;
              userFloat["ppdlTrue3D"] = -99.;
            }
          }
        } else {
          userInt["momPDGId"] =  0;
          userFloat["ppdlTrue"] = -99.;
          userFloat["ppdlTrue3D"] = -99.;
        }
      } else {
        userInt["momPDGId"] =  0;
        userFloat["ppdlTrue"] = -99.;
        userFloat["ppdlTrue3D"] = -99.;
      }

      userInt["Ntrk"] = Ntrk;

      for (std::map<std::string, int>::iterator i = userInt.begin(); i != userInt.end(); i++) { myCand.addUserInt(i->first , i->second); }
      for (std::map<std::string, float>::iterator i = userFloat.begin(); i != userFloat.end(); i++) { myCand.addUserFloat(i->first , i->second); }
      for (std::map<std::string, reco::Vertex>::iterator i = userVertex.begin(); i != userVertex.end(); i++) { myCand.addUserData(i->first , i->second); }

      // ---- Push back output ----  
      oniaOutput->push_back(myCand);
    }
  }

  //  std::sort(oniaOutput->begin(),oniaOutput->end(),pTComparator_);
  std::sort(oniaOutput->begin(),oniaOutput->end(),vPComparator_);

  iEvent.put(oniaOutput);

}


bool 
HiOnia2MuMuPAT::isAbHadron(int pdgID) {

  if (abs(pdgID) == 511 || abs(pdgID) == 521 || abs(pdgID) == 531 || abs(pdgID) == 5122) return true;
  return false;

}

bool 
HiOnia2MuMuPAT::isAMixedbHadron(int pdgID, int momPdgID) {

  if ((abs(pdgID) == 511 && abs(momPdgID) == 511 && pdgID*momPdgID < 0) || 
      (abs(pdgID) == 531 && abs(momPdgID) == 531 && pdgID*momPdgID < 0)) 
      return true;
  return false;

}

reco::GenParticleRef   
HiOnia2MuMuPAT::findMotherRef(reco::GenParticleRef GenParticle, int GenParticlePDG) {

  reco::GenParticleRef GenParticleMother = GenParticle;       // find mothers
  for(int i=0; i<1000; ++i) {
    if (GenParticleMother.isNonnull() && (GenParticleMother->pdgId()==GenParticlePDG) && GenParticleMother->numberOfMothers()>0) {        
      GenParticleMother = GenParticleMother->motherRef();
    } else break;
  }
  return GenParticleMother;

}

std::pair<int, std::pair<float, float> >   
HiOnia2MuMuPAT::findJpsiMCInfo(reco::GenParticleRef genJpsi) {

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
    } else if (Jpsimom->numberOfMothers()<=0) {
      if (isAbHadron(Jpsimom->pdgId())) {  
        momJpsiID = Jpsimom->pdgId();
        trueVtxMom.SetXYZ(Jpsimom->vertex().x(),Jpsimom->vertex().y(),Jpsimom->vertex().z());
        aBhadron = true;
      }
    } else {
      reco::GenParticleRef Jpsigrandmom = findMotherRef(Jpsimom->motherRef(), Jpsimom->pdgId());  
      if (isAbHadron(Jpsimom->pdgId())) {       
        if (Jpsigrandmom.isNonnull() && isAMixedbHadron(Jpsimom->pdgId(),Jpsigrandmom->pdgId())) {       
          momJpsiID = Jpsigrandmom->pdgId();
          trueVtxMom.SetXYZ(Jpsigrandmom->vertex().x(),Jpsigrandmom->vertex().y(),Jpsigrandmom->vertex().z());
        } else {                  
          momJpsiID = Jpsimom->pdgId();
          trueVtxMom.SetXYZ(Jpsimom->vertex().x(),Jpsimom->vertex().y(),Jpsimom->vertex().z());
        }
        aBhadron = true;
      } else if (Jpsigrandmom.isNonnull() && isAbHadron(Jpsigrandmom->pdgId()))  {        
        if (Jpsigrandmom->numberOfMothers()<=0) {
          momJpsiID = Jpsigrandmom->pdgId();
          trueVtxMom.SetXYZ(Jpsigrandmom->vertex().x(),Jpsigrandmom->vertex().y(),Jpsigrandmom->vertex().z());
        } else { 
          reco::GenParticleRef JpsiGrandgrandmom = findMotherRef(Jpsigrandmom->motherRef(), Jpsigrandmom->pdgId());
          if (JpsiGrandgrandmom.isNonnull() && isAMixedbHadron(Jpsigrandmom->pdgId(),JpsiGrandgrandmom->pdgId())) {
            momJpsiID = JpsiGrandgrandmom->pdgId();
            trueVtxMom.SetXYZ(JpsiGrandgrandmom->vertex().x(),JpsiGrandgrandmom->vertex().y(),JpsiGrandgrandmom->vertex().z());
          } else {
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

// ------------ method called once each job just before starting event loop  ------------
void 
HiOnia2MuMuPAT::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HiOnia2MuMuPAT::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(HiOnia2MuMuPAT);
