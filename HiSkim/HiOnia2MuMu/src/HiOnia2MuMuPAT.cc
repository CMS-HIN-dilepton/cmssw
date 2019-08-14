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
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistanceXY.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"     
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
// #include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
// #include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
// #include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
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
  DimuTrkSelection_(iConfig.existsAs<std::string>("DimuTrkSelection") ? iConfig.getParameter<std::string>("DimuTrkSelection") : ""),
  trimuonSelection_(iConfig.existsAs<std::string>("trimuonSelection") ? iConfig.getParameter<std::string>("trimuonSelection") : ""),
  LateDimuonSel_(iConfig.existsAs<std::string>("LateDimuonSel") ? iConfig.getParameter<std::string>("LateDimuonSel") : ""),
  LateDimuTrkSel_(iConfig.existsAs<std::string>("LateDimuTrkSel") ? iConfig.getParameter<std::string>("LateDimuTrkSel") : ""),
  LateTrimuonSel_(iConfig.existsAs<std::string>("LateTrimuonSel") ? iConfig.getParameter<std::string>("LateTrimuonSel") : ""),
  addCommonVertex_(iConfig.getParameter<bool>("addCommonVertex")),
  addMuonlessPrimaryVertex_(iConfig.getParameter<bool>("addMuonlessPrimaryVertex")),
  resolveAmbiguity_(iConfig.getParameter<bool>("resolvePileUpAmbiguity")),
  onlySoftMuons_(iConfig.getParameter<bool>("onlySoftMuons")),
  doTrimuons_(iConfig.getParameter<bool>("doTrimuons")),
  DimuonTrk_(iConfig.getParameter<bool>("DimuonTrk")),
  Converter_(converter::TrackToCandidate(iConfig)),
  trackType_(iConfig.getParameter<int>("particleType"))
{  
  produces<pat::CompositeCandidateCollection>("");
  produces<pat::CompositeCandidateCollection>("trimuon");
  produces<pat::CompositeCandidateCollection>("dimutrk");
}


HiOnia2MuMuPAT::~HiOnia2MuMuPAT()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

bool
HiOnia2MuMuPAT::isSoftMuon(const pat::Muon* aMuon) {
  return (
          muon::isGoodMuon(*aMuon, muon::TMOneStationTight) &&
          aMuon->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5   &&
          aMuon->innerTrack()->hitPattern().pixelLayersWithMeasurement()   > 0   &&
          //aMuon->innerTrack()->quality(reco::TrackBase::highPurity) && 
          fabs(aMuon->innerTrack()->dxy(RefVtx)) < 0.3 &&
          fabs(aMuon->innerTrack()->dz(RefVtx)) < 20.
          );
}

// ------------ method called to produce the data  ------------
void
HiOnia2MuMuPAT::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{  
  using namespace edm;
  using namespace std;
  using namespace reco;
  typedef Candidate::LorentzVector LorentzVector;

  if(DimuonTrk_ && doTrimuons_){
    cout<<"FATAL ERROR: DimuonTrk_ and doTrimuons_ cannot be both true ! Change one of them in the config file !"<<endl;
    return;
  }

  vector<double> muMasses, muMasses3, DimuTrkMasses;
  muMasses.push_back( 0.1056583715 );  muMasses.push_back( 0.1056583715 );
  muMasses3.push_back( 0.1056583715 );  muMasses3.push_back( 0.1056583715 );  muMasses3.push_back( 0.1056583715 );
  DimuTrkMasses.push_back( 0.1056583715 );  DimuTrkMasses.push_back( 0.1056583715 );  DimuTrkMasses.push_back( 0.13957018 );

  std::unique_ptr<pat::CompositeCandidateCollection> oniaOutput(new pat::CompositeCandidateCollection);
  std::unique_ptr<pat::CompositeCandidateCollection> trimuOutput(new pat::CompositeCandidateCollection);
  std::unique_ptr<pat::CompositeCandidateCollection> dimutrkOutput(new pat::CompositeCandidateCollection);
  
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

  RefVtx = thePrimaryV.position();
 
  // // ---------------------------
  Handle< View<pat::Muon> > muons;
  iEvent.getByToken(muonsToken_ , muons);

  edm::ESHandle<TransientTrackBuilder> theTTBuilder;
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theTTBuilder);
  KalmanVertexFitter vtxFitter(true);
  //////////////For kinematic constrained fit
  edm::ESHandle<MagneticField> bField;
  iSetup.get<IdealMagneticFieldRecord>().get(bField);
  KinematicParticleFactoryFromTransientTrack pFactory;
  ParticleMass muon_mass = 0.1056583;
  float muon_sigma = 0.0000000001;
  ParticleMass pion_mass = 0.13957018;
  float pion_sigma = 0.0000000001;
  ParticleMass jp_mass = 3.09687;
  // KinematicParticleVertexFitter Kfitter;
  // KinematicParticleFitter KCfitter;
  MultiTrackKinematicConstraint * jpsi_c = new TwoTrackMassKinematicConstraint(jp_mass);
  KinematicConstrainedVertexFitter KCfitter;
  // ParticleMass Bc_mass = 6.2749;
  // float Bc_sigma = 0.0004;
  //  MassKinematicConstraint * Bc_c = new MassKinematicConstraint(Bc_mass,Bc_sigma);

  TrackCollection muonLess; // track collection related to PV, minus the 2 muons (if muonLessPV option is activated)

  Handle<reco::TrackCollection> collTracks;
  iEvent.getByToken(recoTracksToken_,collTracks);
  int Ntrk = -1; 
  std::vector<reco::TrackRef> ourTracks;
  if ( collTracks.isValid() ) {
    Ntrk = 0;
    // for(std::vector<reco::Track>::const_iterator it=collTracks->begin(); it!=collTracks->end(); ++it) {
    //   const reco::Track* track = &(*it);        
    //   if ( track->qualityByName("highPurity") ) { 
    // 	Ntrk++; 
    // 	if (DimuonTrk_){ 
    // 	  track->setMass(0.13957018);//pion mass for all tracks
    // 	  ourTracks.push_back(*track); }
    //   }
    // }
    for(unsigned int tidx=0; tidx<collTracks->size();tidx++) {
      const reco::TrackRef track(collTracks, tidx);        
      if ( track->qualityByName("highPurity") && track->eta()<2.4 && fabs(track->dxy(RefVtx))<0.3 && fabs(track->dz(RefVtx))<20) { 
	Ntrk++; 
	if (DimuonTrk_){ 
	  ourTracks.push_back(track); }
      }
    }
  }

  std::vector<pat::Muon> ourMuons;
  for(View<pat::Muon>::const_iterator it = muons->begin(), itend = muons->end(); it != itend; ++it){
    if ( lowerPuritySelection_(*it) && (!onlySoftMuons_ || isSoftMuon(&(*it))) ){
      ourMuons.push_back(*it);}
  }
  int ourMuNb = ourMuons.size();
  //std::cout<<"number of soft muons = "<<ourMuNb<<std::endl;

  int Bcnb=0;
  //  int Jpsinb=0;
  // JPsi candidates only from muons
  for(int i=0; i<ourMuNb; i++){
    const pat::Muon& it = ourMuons[i];
    for(int j=i+1; j<ourMuNb; j++){
      const pat::Muon& it2 = ourMuons[j];
      // one muon must pass tight quality
      if (!(higherPuritySelection_(it) || higherPuritySelection_(it2))) continue;

      pat::CompositeCandidate myCand;
      // ---- no explicit order defined ----
      myCand.addDaughter(it, "muon1");
      myCand.addDaughter(it2,"muon2"); 

      // ---- define and set candidate's 4momentum  ----  
      LorentzVector jpsi = it.p4() + it2.p4();
      myCand.setP4(jpsi);
      myCand.setCharge(it.charge()+it2.charge());

      std::map< std::string, int > userInt;
      std::map< std::string, float > userFloat;
      std::map< std::string, reco::Vertex > userVertex;
      Vertex theOriginalPV;

      // ---- fit vertex using Tracker tracks (if they have tracks) ----
      if (it.track().isNonnull() && it2.track().isNonnull()) {
        
        //build the dimuon secondary vertex      

	// //Begin Kinematic Constrained Vertex Fit
	// std::vector<RefCountedKinematicParticle> muons;
	// reco::TransientTrack muon1TT(it.track(), &(*bField) );
	// reco::TransientTrack muon2TT(it2.track(), &(*bField) );
	// if(!muon1TT.isValid()) continue;
	// if(!muon2TT.isValid()) continue;

	// float chi = 0.;	float ndf = 0.;
	// muons.push_back(pFactory.particle(muon1TT,muon_mass,chi,ndf,muon_sigma));
	// muons.push_back(pFactory.particle(muon2TT,muon_mass,chi,ndf,muon_sigma));

	// RefCountedKinematicTree jpsiTree = Kfitter.fit(muons);
	// if(!jpsiTree->isValid()) continue;
	// jpsiTree = KCfitter.fit(jpsi_c,jpsiTree);

	// jpsiTree->movePointerToTheTop();
	// RefCountedKinematicVertex jpsiVtx = jpsiTree->currentDecayVertex();
	// //End Kinematic Constrained Vertex Fit

        vector<TransientTrack> t_tks;
        t_tks.push_back(theTTBuilder->build(*it.track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
        t_tks.push_back(theTTBuilder->build(*it2.track())); // otherwise the vertex will have transient refs inside.

	CachingVertex<5> VtxForInvMass = vtxFitter.vertex( t_tks );
        Measurement1D MassWErr = massCalculator.invariantMass( VtxForInvMass, muMasses );
        userFloat["MassErr"] = MassWErr.error();

	//        TransientVertex myVertex = TransientVertex(jpsiVtx->position(),jpsiVtx->error(),t_tks,jpsiVtx->chiSquared(),jpsiVtx->degreesOfFreedom());
	TransientVertex myVertex = vtxFitter.vertex(t_tks);

        if (myVertex.isValid()) {

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

          theOriginalPV = thePrimaryV;

	  // ---- apply the dimuon cut --- This selection is done only here because "resolvePileUpAmbiguity" info is needed for later Trimuon 
	  if(!dimuonSelection_(myCand)) 
	    {goto TrimuonCand;}

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


          muonLess.clear();
          muonLess.reserve(thePrimaryV.tracksSize());
          if( addMuonlessPrimaryVertex_ && thePrimaryV.tracksSize()>2) {
            // Primary vertex matched to the dimuon, now refit it removing the two muons
            //edm::LogWarning("HiOnia2MuMuPAT_addMuonlessPrimaryVertex") << "If muonLessPV is turned on, ctau is calculated with muonLessPV only.\n" ;

            // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
            const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
            const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
            if( thePrimaryV.hasRefittedTracks() ) {
              // Need to go back to the original tracks before taking the key
              std::vector<reco::Track>::const_iterator itRefittedTrack   = thePrimaryV.refittedTracks().begin();
              std::vector<reco::Track>::const_iterator refittedTracksEnd = thePrimaryV.refittedTracks().end();
              for( ; itRefittedTrack != refittedTracksEnd; ++itRefittedTrack ) {
                if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu1->track().key() ) continue;
                if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu2->track().key() ) continue;

                const reco::Track & recoTrack = *(thePrimaryV.originalTrack(*itRefittedTrack));
                muonLess.push_back(recoTrack);
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
                //edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
		//"Still have the original PV: the refit was not done 'cose it is already muonless" << "\n";
              } else if ( muonLess.size()<=1 ){
                edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
                  "Still have the original PV: the refit was not done 'cose there are not enough tracks to do the refit without the muon tracks" << "\n";
              } else {
                edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
                  "Still have the original PV: Something weird just happened, muonLess.size()=" << muonLess.size() << " and thePrimaryV.tracksSize()=" << thePrimaryV.tracksSize() << " ." << "\n";
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
            const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
            const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
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

          if (addMuonlessPrimaryVertex_) {
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
	  }
	  else {
	    userFloat["ppdlOrigPV"] = ctauPV;
	    userFloat["ppdlErrOrigPV"] = ctauErrPV;
	    userFloat["ppdlOrigPV3D"] = ctauPV3D;
	    userFloat["ppdlErrOrigPV3D"] = ctauErrPV3D;
	  }

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
          
          if (addCommonVertex_) {
            userVertex["commonVertex"] = Vertex(myVertex);
          }
        } else {
          userFloat["vNChi2"] = -1;
          userFloat["vProb"] = -1;
          userFloat["vertexWeight"] = -100;
          userFloat["sumPTPV"] = -100;
          userFloat["DCA"] = -10;
          userFloat["ppdlPV"] = -100;
          userFloat["ppdlErrPV"] = -100;
          userFloat["cosAlpha"] = -10;
          userFloat["ppdlBS"] = -100;
          userFloat["ppdlErrBS"] = -100;
          userFloat["ppdlOrigPV"] = -100;
          userFloat["ppdlErrOrigPV"] = -100;
          userFloat["ppdlPV3D"] = -100;
          userFloat["ppdlErrPV3D"] = -100;
          userFloat["cosAlpha3D"] = -10;
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
        userFloat["DCA"] = -10;
        userFloat["ppdlPV"] = -100;
        userFloat["ppdlErrPV"] = -100;
        userFloat["cosAlpha"] = -10;
        userFloat["ppdlBS"] = -100;
        userFloat["ppdlErrBS"] = -100;
        userFloat["ppdlOrigPV"] = -100;
        userFloat["ppdlErrOrigPV"] = -100;
        userFloat["ppdlPV3D"] = -100;
        userFloat["ppdlErrPV3D"] = -100;
        userFloat["cosAlpha3D"] = -10;
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

      userInt["Ntrk"] = Ntrk;

      for (std::map<std::string, int>::iterator i = userInt.begin(); i != userInt.end(); i++) { myCand.addUserInt(i->first , i->second); }
      for (std::map<std::string, float>::iterator i = userFloat.begin(); i != userFloat.end(); i++) { myCand.addUserFloat(i->first , i->second); }
      for (std::map<std::string, reco::Vertex>::iterator i = userVertex.begin(); i != userVertex.end(); i++) { myCand.addUserData(i->first , i->second); }

      if(!LateDimuonSel_(myCand)) continue;
      // ---- Push back output ----  
      oniaOutput->push_back(myCand);
      //**********************************************************
      
      if(!DimuonTrk_) 
	{goto TrimuonCand;}

      //pdtentry_ = PdtEntry(211);
      //pdtentry_.setup(iSetup); //Setup needed for the converter TrackToCandidate (depending on MassiveCandidateConverter)

      for(int k=0; k<Ntrk; k++){
	const reco::TrackRef it3 = ourTracks[k];
	//cout<<"Got track #"<<k<<endl;
	RecoChargedCandidate piCand3;
	Converter_.TrackToCandidate::convert(it3, piCand3);
	piCand3.setPdgId(211);
	piCand3.setMass(0.13957018);//pion mass for all tracks
	
	if( ( fabs((it3->pt() - (it.track())->pt())) < 1e-4 && fabs((it3->eta() - (it.track())->eta())) < 1e-6) || (fabs((it3->pt() - (it2.track())->pt())) < 1e-4 && (fabs(it3->eta() - (it2.track())->eta())) < 1e-6) ) {
	  //cout<<"Skipping track #"<<k<<" because it is the same as one of the two muon tracks"<<endl;
	  continue;}
	pat::CompositeCandidate BcCand;
	// ---- no explicit order defined ----
        BcCand.addDaughter(it, "muon1");
        BcCand.addDaughter(it2,"muon2");
        BcCand.addDaughter(piCand3,"track");

    	// ---- define and set candidate's 4momentum  ----  
    	LorentzVector bc = it.p4() + it2.p4() + piCand3.p4();
    	BcCand.setP4(bc);
    	BcCand.setCharge(it.charge()+it2.charge()+piCand3.charge());

    	std::map< std::string, float > userBcFloat;
    	std::map< std::string, reco::Vertex > userBcVertex;

        // ---- apply the Bc cut ---- 
	if(!( DimuTrkSelection_(BcCand))) continue;

    	// ---- fit vertex using Tracker tracks (if they have tracks) ----
    	if (it.track().isNonnull() && it2.track().isNonnull()) {

    	  //build the Jpsi+trk secondary vertex	  
	  //////////// Kalman Vertex Fitter
    	  vector<TransientTrack> t_tks;
    	  t_tks.push_back(theTTBuilder->build(*it.track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
    	  t_tks.push_back(theTTBuilder->build(*it2.track())); // otherwise the vertex will have transient refs inside.
    	  t_tks.push_back(theTTBuilder->build(*piCand3.track()));

    	  CachingVertex<5> VtxForInvMass = vtxFitter.vertex( t_tks );
    	  Measurement1D MassWErr = massCalculator.invariantMass( VtxForInvMass, DimuTrkMasses );
    	  userBcFloat["MassErr"] = MassWErr.error();

	  TransientVertex DimuTrkVertex = vtxFitter.vertex(t_tks);

    	  if (DimuTrkVertex.isValid()) {
    	    float vChi2 = DimuTrkVertex.totalChiSquared();
    	    float vNDF  = DimuTrkVertex.degreesOfFreedom();
    	    float vProb(TMath::Prob(vChi2,(int)vNDF));

    	    Bcnb+=1;
    	    
	    userBcFloat["vNChi2"] = (vChi2/vNDF);
    	    userBcFloat["vProb"] = vProb;

    	    TVector3 vtx, vtx3D;
    	    TVector3 pvtx, pvtx3D;
    	    VertexDistanceXY vdistXY;
    	    VertexDistance3D vdistXYZ;

    	    vtx.SetXYZ(DimuTrkVertex.position().x(),DimuTrkVertex.position().y(),0);
    	    TVector3 pperp(bc.px(), bc.py(), 0);
    	    AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

    	    vtx3D.SetXYZ(DimuTrkVertex.position().x(),DimuTrkVertex.position().y(),DimuTrkVertex.position().z());
    	    TVector3 pxyz(bc.px(), bc.py(), bc.pz());
    	    AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

    	    //The "resolvePileUpAmbiguity" (looking for the PV that is the closest in z to the displaced vertex) has already been done with the dimuon, we keep this PV as such
    	    Vertex thePrimaryV = theOriginalPV;
	    
    	    muonLess.clear();
    	    muonLess.reserve(thePrimaryV.tracksSize());
    	    if( addMuonlessPrimaryVertex_ && thePrimaryV.tracksSize()>2) {
    	      // Primary vertex matched to the dimuon, now refit it removing the three muons
    	      // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
    	      const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
    	      const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
    	      const reco::TrackRef rtrk3 = it3;
    	      if( thePrimaryV.hasRefittedTracks() ) {
    		// Need to go back to the original tracks before taking the key
    		std::vector<reco::Track>::const_iterator itRefittedTrack   = thePrimaryV.refittedTracks().begin();
    		std::vector<reco::Track>::const_iterator refittedTracksEnd = thePrimaryV.refittedTracks().end();
    		for( ; itRefittedTrack != refittedTracksEnd; ++itRefittedTrack ) {
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu1->track().key() ) continue;
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu2->track().key() ) continue;
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rtrk3.key() ) continue;

    		  const reco::Track & recoTrack = *(thePrimaryV.originalTrack(*itRefittedTrack));
    		  muonLess.push_back(recoTrack);
    		}
    	      }// PV has refitted tracks
    	      else 
    		{
    		  std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
    		  for( ; itPVtrack != thePrimaryV.tracks_end(); ++itPVtrack ) if (itPVtrack->isNonnull()) {
    		      if( itPVtrack->key() == rmu1->track().key() ) continue;
    		      if( itPVtrack->key() == rmu2->track().key() ) continue;
    		      if( itPVtrack->key() == rtrk3.key() ) continue;
    		      muonLess.push_back(**itPVtrack);
    		    }
    		}// take all tracks associated with the vtx

    	      if (muonLess.size()>1 && muonLess.size() < thePrimaryV.tracksSize()) {
    		// find the new vertex, from which the 2 muons were removed
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
    		  // edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
    		  //  "Still have the original PV: the refit was not done 'cose there are not enough tracks to do the refit without the muon tracks" << "\n";
    		} else {
    		  edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
    		    "Still have the original PV: Something weird just happened, muonLess.size()=" << muonLess.size() << " and thePrimaryV.tracksSize()=" << thePrimaryV.tracksSize() << " ." << "\n";
    		}
    	      }
    	    }// refit vtx without the muon tracks

    	    if (addMuonlessPrimaryVertex_) {
    	      userBcVertex["muonlessPV"] = thePrimaryV;
    	      userBcVertex["PVwithmuons"] = theOriginalPV;
    	    } else {
    	      userBcVertex["PVwithmuons"] = thePrimaryV;
    	    }

    	    // lifetime using PV
    	    pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
    	    TVector3 vdiff = vtx - pvtx;
    	    double cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
    	    Measurement1D distXY = vdistXY.distance(Vertex(DimuTrkVertex), thePrimaryV);
    	    double ctauPV = distXY.value()*cosAlpha*6.276/pperp.Perp();
    	    GlobalError v1e = (Vertex(DimuTrkVertex)).error();
    	    GlobalError v2e = thePrimaryV.error();
    	    AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
    	    double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*6.276/(pperp.Perp2());

    	    userBcFloat["ppdlPV"] = ctauPV;
    	    userBcFloat["ppdlErrPV"] = ctauErrPV;
    	    userBcFloat["cosAlpha"] = cosAlpha;

    	    pvtx3D.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),thePrimaryV.position().z());
    	    TVector3 vdiff3D = vtx3D - pvtx3D;
    	    double cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
    	    Measurement1D distXYZ = vdistXYZ.distance(Vertex(DimuTrkVertex), thePrimaryV);
    	    double ctauPV3D = distXYZ.value()*cosAlpha3D*6.276/pxyz.Mag();
    	    double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYe))*6.276/(pxyz.Mag2());
          
    	    userBcFloat["ppdlPV3D"] = ctauPV3D;
    	    userBcFloat["ppdlErrPV3D"] = ctauErrPV3D;
    	    userBcFloat["cosAlpha3D"] = cosAlpha3D;

    	    if (addCommonVertex_) {
    	      userBcVertex["commonVertex"] = Vertex(DimuTrkVertex);
    	    }

	  } else {
	    userBcFloat["vNChi2"] = -1;
	    userBcFloat["vProb"] = -1;
	    userBcFloat["vertexWeight"] = -100;
	    userBcFloat["sumPTPV"] = -100;
	    userBcFloat["ppdlPV"] = -100;
	    userBcFloat["ppdlErrPV"] = -100;
	    userBcFloat["cosAlpha"] = -100;
	    userBcFloat["ppdlBS"] = -100;
	    userBcFloat["ppdlErrBS"] = -100;
	    userBcFloat["ppdlOrigPV"] = -100;
	    userBcFloat["ppdlErrOrigPV"] = -100;
	    userBcFloat["ppdlPV3D"] = -100;
	    userBcFloat["ppdlErrPV3D"] = -100;
	    userBcFloat["cosAlpha3D"] = -100;
	    userBcFloat["ppdlBS3D"] = -100;
	    userBcFloat["ppdlErrBS3D"] = -100;
	    userBcFloat["ppdlOrigPV3D"] = -100;
	    userBcFloat["ppdlErrOrigPV3D"] = -100;

    	    if (addCommonVertex_) {
    	      userBcVertex["commonVertex"] = Vertex();
    	    }
    	    if (addMuonlessPrimaryVertex_) {
    	      userBcVertex["muonlessPV"] = Vertex();
    	      userBcVertex["PVwithmuons"] = Vertex();
    	    } else {
    	      userBcVertex["PVwithmuons"] = Vertex();
    	    } 
	  }	  
	} else {
	  userBcFloat["vNChi2"] = -1;
	  userBcFloat["vProb"] = -1;
	  userBcFloat["vertexWeight"] = -100;
	  userBcFloat["sumPTPV"] = -100;
	  userBcFloat["ppdlPV"] = -100;
	  userBcFloat["ppdlErrPV"] = -100;
	  userBcFloat["cosAlpha"] = -100;
	  userBcFloat["ppdlBS"] = -100;
	  userBcFloat["ppdlErrBS"] = -100;
	  userBcFloat["ppdlOrigPV"] = -100;
	  userBcFloat["ppdlErrOrigPV"] = -100;
	  userBcFloat["ppdlPV3D"] = -100;
	  userBcFloat["ppdlErrPV3D"] = -100;
	  userBcFloat["cosAlpha3D"] = -100;
	  userBcFloat["ppdlBS3D"] = -100;
	  userBcFloat["ppdlErrBS3D"] = -100;
	  userBcFloat["ppdlOrigPV3D"] = -100;
	  userBcFloat["ppdlErrOrigPV3D"] = -100;

    	  if (addCommonVertex_) {
    	    userBcVertex["commonVertex"] = Vertex();
    	  }
    	  if (addMuonlessPrimaryVertex_) {
    	    userBcVertex["muonlessPV"] = Vertex();
    	    userBcVertex["PVwithmuons"] = Vertex();
    	  } else {
    	    userBcVertex["PVwithmuons"] = Vertex();
    	  }
	}

	for (std::map<std::string, float>::iterator i = userBcFloat.begin(); i != userBcFloat.end(); i++) { BcCand.addUserFloat(i->first , i->second); }
    	for (std::map<std::string, reco::Vertex>::iterator i = userBcVertex.begin(); i != userBcVertex.end(); i++) { BcCand.addUserData(i->first , i->second); }

	if(!LateDimuTrkSel_(BcCand)) continue;	

	bool KCvtxNotFound=true;
    	if (it.track().isNonnull() && it2.track().isNonnull()) {
	  /////////////////Begin Kinematic Constrained Vertex Fit
	  std::vector<RefCountedKinematicParticle> BcDaughters;
	  reco::TransientTrack muon1TT(it.track(), &(*bField) );
	  reco::TransientTrack muon2TT(it2.track(), &(*bField) );
	  reco::TransientTrack pion3TT(piCand3.track(), &(*bField) );

	  if(muon1TT.isValid() && muon2TT.isValid() && pion3TT.isValid()) {
	    float chi = 0.;	float ndf = 0.;
	    BcDaughters.push_back(pFactory.particle(muon1TT,muon_mass,chi,ndf,muon_sigma));
	    BcDaughters.push_back(pFactory.particle(muon2TT,muon_mass,chi,ndf,muon_sigma));
	    BcDaughters.push_back(pFactory.particle(pion3TT,pion_mass,chi,ndf,pion_sigma));

	    RefCountedKinematicTree BcTree = KCfitter.fit(BcDaughters, jpsi_c);   
	    if(BcTree->isValid()){
	      BcTree->movePointerToTheTop();
	      RefCountedKinematicParticle BcPart = BcTree->currentParticle();
	      RefCountedKinematicVertex BcVtx = BcTree->currentDecayVertex();
	      if(BcVtx->vertexIsValid()){
		KCvtxNotFound=false;
		//////////////////End Kinematic Constrained Vertex Fit

		double vtxProb = TMath::Prob(BcVtx->chiSquared(), BcVtx->degreesOfFreedom());
		userBcFloat["KinConstrainedVtxProb"] = vtxProb;

		// lifetime using PV and vertex from kin constrained fit
		TVector3 vtx, vtx3D;
		TVector3 pvtx, pvtx3D;
		VertexDistanceXY vdistXY;
		VertexDistance3D vdistXYZ;

		vtx.SetXYZ(BcVtx->position().x(),BcVtx->position().y(),0);
		TVector3 pperp(BcPart->currentState().kinematicParameters().momentum().x(), BcPart->currentState().kinematicParameters().momentum().y(), 0);
		AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

		vtx3D.SetXYZ(vtx.X(), vtx.Y(), BcVtx->position().z());
		TVector3 pxyz(pperp.x(), pperp.y(), BcPart->currentState().kinematicParameters().momentum().z());
		AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

		pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
		TVector3 vdiff = vtx - pvtx;
		double cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
		Measurement1D distXY = vdistXY.distance(Vertex(
							       math::XYZPoint(vtx3D.X(), vtx3D.Y(), vtx3D.Z()),
							       reco::Vertex::Error()), thePrimaryV); //!!! Put 0 error here because we use only dist.value
		double ctauPV = distXY.value()*cosAlpha*6.275/pperp.Perp();
		GlobalError v1e = BcVtx->error();
		GlobalError v2e = thePrimaryV.error();
		AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
		double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*6.275/(pperp.Perp2());

		userBcFloat["KCppdlPV"] = ctauPV;
		userBcFloat["KCppdlErrPV"] = ctauErrPV;
		userBcFloat["KCcosAlpha"] = cosAlpha;

		pvtx3D.SetXYZ(pvtx.X(),pvtx.Y(),thePrimaryV.position().z());
		TVector3 vdiff3D = vtx3D - pvtx3D;
		double cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
		Measurement1D distXYZ = vdistXYZ.distance(Vertex(
								 math::XYZPoint(vtx3D.X(), vtx3D.Y(), vtx3D.Z()),
								 reco::Vertex::Error()), thePrimaryV); //!!! Put 0 error here because we use only dist.value
		double ctauPV3D = distXYZ.value()*cosAlpha3D*6.275/pxyz.Mag();
		double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYe))*6.275/(pxyz.Mag2());
          
		userBcFloat["KCppdlPV3D"] = ctauPV3D;
		userBcFloat["KCppdlErrPV3D"] = ctauErrPV3D;
		userBcFloat["KCcosAlpha3D"] = cosAlpha3D;

	      }
	    }
	  }
	}
	if(KCvtxNotFound) {
	  userBcFloat["KinConstrainedVtxProb"] = -1;
	  userBcFloat["KCppdlPV"] = -10;
	  userBcFloat["KCppdlErrPV"] = -10;
	  userBcFloat["KCcosAlpha"] = -10;
	  userBcFloat["KCppdlPV3D"] = -10;
	  userBcFloat["KCppdlErrPV3D"] = -10;
	  userBcFloat["KCcosAlpha3D"] = -10;
	}
	for (std::map<std::string, float>::iterator i = userBcFloat.begin(); i != userBcFloat.end(); i++) { BcCand.addUserFloat(i->first , i->second); }
	//	BcCand.addUserFloat("KinConstrainedVtxProb",userBcFloat["KinConstrainedVtxProb"]);

	// ---- Push back output ----  
	dimutrkOutput->push_back(BcCand);
      }//it3 end loop
    
    TrimuonCand:
      if(!doTrimuons_) continue;
      // ---- Create all trimuon combinations (Bc candidates) ----
      for(int k=j+1; k<ourMuNb; k++){
      	const pat::Muon& it3 = ourMuons[k];
    	// Two must pass tight quality  (includes |eta|<2.4)
    	if (!( (higherPuritySelection_(it) && higherPuritySelection_(it2))
	       || (higherPuritySelection_(it) && higherPuritySelection_(it3))
	       || (higherPuritySelection_(it2) && higherPuritySelection_(it3)) )) continue;

    	pat::CompositeCandidate BcCand;
    	// ---- no explicit order defined ----
    	BcCand.addDaughter(it, "muon1");
    	BcCand.addDaughter(it2,"muon2"); 
    	BcCand.addDaughter(it3,"muon3"); 
 
    	// ---- define and set candidate's 4momentum  ----  
    	LorentzVector bc = it.p4() + it2.p4() + it3.p4();
    	BcCand.setP4(bc);
    	BcCand.setCharge(it.charge()+it2.charge()+it3.charge());

    	std::map< std::string, int > userBcInt;
    	std::map< std::string, float > userBcFloat;
    	std::map< std::string, reco::Vertex > userBcVertex;

    	// ---- Redefine the three possible Jpsi's, to apply dimuon cuts to one of them ----
    	pat::CompositeCandidate myCand2;
    	myCand2.addDaughter(it,"muon1"); 
    	myCand2.addDaughter(it3,"muon2"); 
    	LorentzVector jpsi2 = it.p4() + it3.p4();
    	myCand2.setP4(jpsi2);

    	pat::CompositeCandidate myCand3;
    	myCand3.addDaughter(it2,"muon1"); 
    	myCand3.addDaughter(it3,"muon2"); 
    	LorentzVector jpsi3 = it2.p4() + it3.p4();
    	myCand3.setP4(jpsi3);

	// ---- apply the trimuon cut ----
	if(!( trimuonSelection_(BcCand)
	      && (dimuonSelection_(myCand) || dimuonSelection_(myCand2) || dimuonSelection_(myCand3)) )
	   ) continue;
	
    	// ---- fit vertex using Tracker tracks (if they have tracks) ----
    	if (it.track().isNonnull() && it2.track().isNonnull() && it3.track().isNonnull()) {
	  
    	  //build the trimuon secondary vertex
	  
    	  vector<TransientTrack> t_tks;
    	  t_tks.push_back(theTTBuilder->build(*it.track()));  // pass the reco::Track, not  the reco::TrackRef (which can be transient)
    	  t_tks.push_back(theTTBuilder->build(*it2.track())); // otherwise the vertex will have transient refs inside.
    	  t_tks.push_back(theTTBuilder->build(*it3.track()));
    	  TransientVertex TrimuVertex = vtxFitter.vertex(t_tks);
	  
    	  CachingVertex<5> VtxForInvMass = vtxFitter.vertex( t_tks );
    	  Measurement1D MassWErr = massCalculator.invariantMass( VtxForInvMass, muMasses3 );
    	  userBcFloat["MassErr"] = MassWErr.error();

    	  if (TrimuVertex.isValid()) {
    	    float vChi2 = TrimuVertex.totalChiSquared();
    	    float vNDF  = TrimuVertex.degreesOfFreedom();
    	    float vProb(TMath::Prob(vChi2,(int)vNDF));

    	    Bcnb+=1;
    	    
	    userBcFloat["vNChi2"] = (vChi2/vNDF);
    	    userBcFloat["vProb"] = vProb;

    	    TVector3 vtx, vtx3D;
    	    TVector3 pvtx, pvtx3D;
    	    VertexDistanceXY vdistXY;
    	    VertexDistance3D vdistXYZ;

    	    vtx.SetXYZ(TrimuVertex.position().x(),TrimuVertex.position().y(),0);
    	    TVector3 pperp(bc.px(), bc.py(), 0);
    	    AlgebraicVector3 vpperp(pperp.x(), pperp.y(), 0.);

    	    vtx3D.SetXYZ(TrimuVertex.position().x(),TrimuVertex.position().y(),TrimuVertex.position().z());
    	    TVector3 pxyz(bc.px(), bc.py(), bc.pz());
    	    AlgebraicVector3 vpxyz(pxyz.x(), pxyz.y(), pxyz.z());

    	    //The "resolvePileUpAmbiguity" (looking for the PV that is the closest in z to the displaced vertex) has already been done with the dimuon, we keep this PV as such
    	    Vertex thePrimaryV = theOriginalPV;

    	    muonLess.clear();
    	    muonLess.reserve(thePrimaryV.tracksSize());
    	    if( addMuonlessPrimaryVertex_ && thePrimaryV.tracksSize()>3) {
    	      // Primary vertex matched to the trimuon, now refit it removing the three muons
    	      // I need to go back to the reco::Muon object, as the TrackRef in the pat::Muon can be an embedded ref.
    	      const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
    	      const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
    	      const reco::Muon *rmu3 = dynamic_cast<const reco::Muon *>(it3.originalObject());
    	      if( thePrimaryV.hasRefittedTracks() ) {
    		// Need to go back to the original tracks before taking the key
    		std::vector<reco::Track>::const_iterator itRefittedTrack   = thePrimaryV.refittedTracks().begin();
    		std::vector<reco::Track>::const_iterator refittedTracksEnd = thePrimaryV.refittedTracks().end();
    		for( ; itRefittedTrack != refittedTracksEnd; ++itRefittedTrack ) {
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu1->track().key() ) continue;
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu2->track().key() ) continue;
    		  if( thePrimaryV.originalTrack(*itRefittedTrack).key() == rmu3->track().key() ) continue;

    		  const reco::Track & recoTrack = *(thePrimaryV.originalTrack(*itRefittedTrack));
    		  muonLess.push_back(recoTrack);
    		}
    	      }// PV has refitted tracks
    	      else 
    		{
    		  std::vector<reco::TrackBaseRef>::const_iterator itPVtrack = thePrimaryV.tracks_begin();
    		  for( ; itPVtrack != thePrimaryV.tracks_end(); ++itPVtrack ) if (itPVtrack->isNonnull()) {
    		      if( itPVtrack->key() == rmu1->track().key() ) continue;
    		      if( itPVtrack->key() == rmu2->track().key() ) continue;
    		      if( itPVtrack->key() == rmu3->track().key() ) continue;
    		      muonLess.push_back(**itPVtrack);
    		    }
    		}// take all tracks associated with the vtx

    	      if (muonLess.size()>1 && muonLess.size() < thePrimaryV.tracksSize()) {
    		// find the new vertex, from which the 2 muons were removed
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
    		  //edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
    		  //  "Still have the original PV: the refit was not done 'cose it is already muonless" << "\n";
    		} else if ( muonLess.size()<=1 ){
    		  //edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
    		  //  "Still have the original PV: the refit was not done 'cose there are not enough tracks to do the refit without the muon tracks" << "\n";
    		} else {
    		  edm::LogWarning("HiOnia2MuMuPAT_muonLessSizeORpvTrkSize") << 
    		    "Still have the original PV: Something weird just happened, muonLess.size()=" << muonLess.size() << " and thePrimaryV.tracksSize()=" << thePrimaryV.tracksSize() << " ." << "\n";
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
    	      const reco::Muon *rmu1 = dynamic_cast<const reco::Muon *>(it.originalObject());
    	      const reco::Muon *rmu2 = dynamic_cast<const reco::Muon *>(it2.originalObject());
    	      const reco::Muon *rmu3 = dynamic_cast<const reco::Muon *>(it3.originalObject());
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

    	    	    // do not count the three muons
    	    	    if (rmu1 != 0 && rmu1->innerTrack().key() == itVtx->key()) continue;
    	    	    if (rmu2 != 0 && rmu2->innerTrack().key() == itVtx->key()) continue;
    	    	    if (rmu3 != 0 && rmu3->innerTrack().key() == itVtx->key()) continue;

    	    	    vertexWeight += theOriginalPV.trackWeight(*itVtx);
    	    	    if(theOriginalPV.trackWeight(*itVtx) > 0.5){
    	    	      countTksOfPV++;
    	    	      sumPTPV += track.pt();
    	    	    }
    	    	  }
    	      } catch (std::exception & err) {std::cout << " Counting tracks from PV, fails! " << std::endl; return ; }
    	    }
    	    userBcInt["countTksOfPV"] = countTksOfPV;
    	    userBcFloat["vertexWeight"] = (float) vertexWeight;
    	    userBcFloat["sumPTPV"] = (float) sumPTPV;
         
    	    if (addMuonlessPrimaryVertex_) {
    	      userBcVertex["muonlessPV"] = thePrimaryV;
    	      userBcVertex["PVwithmuons"] = theOriginalPV;
    	    } else {
    	      userBcVertex["PVwithmuons"] = thePrimaryV;
    	    }

    	    // lifetime using PV
    	    pvtx.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),0);
    	    TVector3 vdiff = vtx - pvtx;
    	    double cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
    	    Measurement1D distXY = vdistXY.distance(Vertex(TrimuVertex), thePrimaryV);
    	    double ctauPV = distXY.value()*cosAlpha*6.275/pperp.Perp();
    	    GlobalError v1e = (Vertex(TrimuVertex)).error();
    	    GlobalError v2e = thePrimaryV.error();
    	    AlgebraicSymMatrix33 vXYe = v1e.matrix()+ v2e.matrix();
    	    double ctauErrPV = sqrt(ROOT::Math::Similarity(vpperp,vXYe))*6.276/(pperp.Perp2());

    	    userBcFloat["ppdlPV"] = ctauPV;
    	    userBcFloat["ppdlErrPV"] = ctauErrPV;
    	    userBcFloat["cosAlpha"] = cosAlpha;

    	    pvtx3D.SetXYZ(thePrimaryV.position().x(),thePrimaryV.position().y(),thePrimaryV.position().z());
    	    TVector3 vdiff3D = vtx3D - pvtx3D;
    	    double cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
    	    Measurement1D distXYZ = vdistXYZ.distance(Vertex(TrimuVertex), thePrimaryV);
    	    double ctauPV3D = distXYZ.value()*cosAlpha3D*6.275/pxyz.Mag();
    	    double ctauErrPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYe))*6.276/(pxyz.Mag2());
          
    	    userBcFloat["ppdlPV3D"] = ctauPV3D;
    	    userBcFloat["ppdlErrPV3D"] = ctauErrPV3D;
    	    userBcFloat["cosAlpha3D"] = cosAlpha3D;

	    // if (addMuonlessPrimaryVertex_) {
	    //   // lifetime using Original PV
	    //   pvtx.SetXYZ(theOriginalPV.position().x(),theOriginalPV.position().y(),0);
	    //   vdiff = vtx - pvtx;
	    //   double cosAlphaOrigPV = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
	    //   distXY = vdistXY.distance(Vertex(TrimuVertex), theOriginalPV);
	    //   double ctauOrigPV = distXY.value()*cosAlphaOrigPV*6.276/pperp.Perp();
	    //   GlobalError v1eOrigPV = (Vertex(TrimuVertex)).error();
	    //   GlobalError v2eOrigPV = theOriginalPV.error();
	    //   AlgebraicSymMatrix33 vXYeOrigPV = v1eOrigPV.matrix()+ v2eOrigPV.matrix();
	    //   double ctauErrOrigPV = sqrt(ROOT::Math::Similarity(vpperp,vXYeOrigPV))*6.276/(pperp.Perp2());

	    //   userBcFloat["ppdlOrigPV"] = ctauOrigPV;
	    //   userBcFloat["ppdlErrOrigPV"] = ctauErrOrigPV;

	    //   pvtx3D.SetXYZ(theOriginalPV.position().x(), theOriginalPV.position().y(), theOriginalPV.position().z());
	    //   vdiff3D = vtx3D - pvtx3D;
	    //   double cosAlphaOrigPV3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
	    //   distXYZ = vdistXYZ.distance(Vertex(TrimuVertex), theOriginalPV);
	    //   double ctauOrigPV3D = distXYZ.value()*cosAlphaOrigPV3D*6.276/pxyz.Mag();
	    //   double ctauErrOrigPV3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYeOrigPV))*6.276/(pxyz.Mag2());
          
	    //   userBcFloat["ppdlOrigPV3D"] = ctauOrigPV3D;
	    //   userBcFloat["ppdlErrOrigPV3D"] = ctauErrOrigPV3D;
	    // }
	    // else{
	    //   userBcFloat["ppdlOrigPV"] = ctauPV;
	    //   userBcFloat["ppdlErrOrigPV"] = ctauErrPV;
	    //   userBcFloat["ppdlOrigPV3D"] = ctauPV3D;
	    //   userBcFloat["ppdlErrOrigPV3D"] = ctauErrPV3D;	      
	    // }

    	    // // lifetime using BS
    	    // pvtx.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),0);
    	    // vdiff = vtx - pvtx;
    	    // cosAlpha = vdiff.Dot(pperp)/(vdiff.Perp()*pperp.Perp());
    	    // distXY = vdistXY.distance(Vertex(TrimuVertex), theBeamSpotV);
    	    // double ctauBS = distXY.value()*cosAlpha*6.276/pperp.Perp();
    	    // GlobalError v1eB = (Vertex(TrimuVertex)).error();
    	    // GlobalError v2eB = theBeamSpotV.error();
    	    // AlgebraicSymMatrix33 vXYeB = v1eB.matrix()+ v2eB.matrix();
    	    // double ctauErrBS = sqrt(ROOT::Math::Similarity(vpperp,vXYeB))*6.276/(pperp.Perp2());
          
    	    // userBcFloat["ppdlBS"] = ctauBS;
    	    // userBcFloat["ppdlErrBS"] = ctauErrBS;
    	    // pvtx3D.SetXYZ(theBeamSpotV.position().x(),theBeamSpotV.position().y(),theBeamSpotV.position().z());
    	    // vdiff3D = vtx3D - pvtx3D;
    	    // cosAlpha3D = vdiff3D.Dot(pxyz)/(vdiff3D.Mag()*pxyz.Mag());
    	    // distXYZ = vdistXYZ.distance(Vertex(TrimuVertex), theBeamSpotV);
    	    // double ctauBS3D = distXYZ.value()*cosAlpha3D*6.276/pxyz.Mag();
    	    // double ctauErrBS3D = sqrt(ROOT::Math::Similarity(vpxyz,vXYeB))*6.276/(pxyz.Mag2());

    	    // userBcFloat["ppdlBS3D"] = ctauBS3D;
    	    // userBcFloat["ppdlErrBS3D"] = ctauErrBS3D;
          
    	    if (addCommonVertex_) {
    	      userBcVertex["commonVertex"] = Vertex(TrimuVertex);
    	    }

    	  } else {
    	    userBcFloat["vNChi2"] = -1;
    	    userBcFloat["vProb"] = -1;
    	    userBcFloat["vertexWeight"] = -100;
    	    userBcFloat["sumPTPV"] = -100;
    	    userBcFloat["ppdlPV"] = -100;
    	    userBcFloat["ppdlErrPV"] = -100;
    	    userBcFloat["cosAlpha"] = -100;
    	    userBcFloat["ppdlBS"] = -100;
    	    userBcFloat["ppdlErrBS"] = -100;
    	    userBcFloat["ppdlOrigPV"] = -100;
    	    userBcFloat["ppdlErrOrigPV"] = -100;
    	    userBcFloat["ppdlPV3D"] = -100;
    	    userBcFloat["ppdlErrPV3D"] = -100;
    	    userBcFloat["cosAlpha3D"] = -100;
    	    userBcFloat["ppdlBS3D"] = -100;
    	    userBcFloat["ppdlErrBS3D"] = -100;
    	    userBcFloat["ppdlOrigPV3D"] = -100;
    	    userBcFloat["ppdlErrOrigPV3D"] = -100;

    	    userBcInt["countTksOfPV"] = -1;

    	    if (addCommonVertex_) {
    	      userBcVertex["commonVertex"] = Vertex();
    	    }
    	    if (addMuonlessPrimaryVertex_) {
    	      userBcVertex["muonlessPV"] = Vertex();
    	      userBcVertex["PVwithmuons"] = Vertex();
    	    } else {
    	      userBcVertex["PVwithmuons"] = Vertex();
    	    } 
    	  }
    	} else {
    	  userBcFloat["vNChi2"] = -1;
    	  userBcFloat["vProb"] = -1;
    	  userBcFloat["vertexWeight"] = -100;
    	  userBcFloat["sumPTPV"] = -100;
    	  userBcFloat["ppdlPV"] = -100;
    	  userBcFloat["ppdlErrPV"] = -100;
    	  userBcFloat["cosAlpha"] = -100;
    	  userBcFloat["ppdlBS"] = -100;
    	  userBcFloat["ppdlErrBS"] = -100;
    	  userBcFloat["ppdlOrigPV"] = -100;
    	  userBcFloat["ppdlErrOrigPV"] = -100;
    	  userBcFloat["ppdlPV3D"] = -100;
    	  userBcFloat["ppdlErrPV3D"] = -100;
    	  userBcFloat["cosAlpha3D"] = -100;
    	  userBcFloat["ppdlBS3D"] = -100;
    	  userBcFloat["ppdlErrBS3D"] = -100;
    	  userBcFloat["ppdlOrigPV3D"] = -100;
    	  userBcFloat["ppdlErrOrigPV3D"] = -100;
        
    	  userBcInt["countTksOfPV"] = -1;
        
    	  if (addCommonVertex_) {
    	    userBcVertex["commonVertex"] = Vertex();
    	  }
    	  if (addMuonlessPrimaryVertex_) {
    	    userBcVertex["muonlessPV"] = Vertex();
    	    userBcVertex["PVwithmuons"] = Vertex();
    	  } else {
    	    userBcVertex["PVwithmuons"] = Vertex();
    	  }
    	}

    	for (std::map<std::string, int>::iterator i = userBcInt.begin(); i != userBcInt.end(); i++) { BcCand.addUserInt(i->first , i->second); }
    	for (std::map<std::string, float>::iterator i = userBcFloat.begin(); i != userBcFloat.end(); i++) { BcCand.addUserFloat(i->first , i->second); }
    	for (std::map<std::string, reco::Vertex>::iterator i = userBcVertex.begin(); i != userBcVertex.end(); i++) { BcCand.addUserData(i->first , i->second); }

	if(!LateTrimuonSel_(BcCand)) continue;	
    	// ---- Push back output ----  
    	trimuOutput->push_back(BcCand);

      }//it3 muon
    }//it2 muon
  }//it muon

  //  std::sort(oniaOutput->begin(),oniaOutput->end(),pTComparator_);
  std::sort(oniaOutput->begin(),oniaOutput->end(),vPComparator_);
  iEvent.put(std::move(oniaOutput),"");

  if(doTrimuons_){
    std::sort(trimuOutput->begin(),trimuOutput->end(),vPComparator_);
    iEvent.put(std::move(trimuOutput),"trimuon");
  }

  if(DimuonTrk_){
    std::sort(dimutrkOutput->begin(),dimutrkOutput->end(),vPComparator_);
    iEvent.put(std::move(dimutrkOutput),"dimutrk");
  }
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
