// -*- C++ -*-
//
// Package:    MLAnalyzer/RecHitAnalyzer
// Class:      RecHitAnalyzer
// 
//
// Original Author:  Michael Andrews
//         Created:  Sat, 14 Jan 2017 17:45:54 GMT
//
//
#include "MLAnalyzer/RecHitAnalyzer/interface/RecHitAnalyzer.h"

//
// constructors and destructor
//
RecHitAnalyzer::RecHitAnalyzer(const edm::ParameterSet& iConfig)
{
  //EBRecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("EBRecHitCollection"));
  EBRecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("reducedEBRecHitCollection"));
  //EBDigiCollectionT_ = consumes<EBDigiCollection>(iConfig.getParameter<edm::InputTag>("selectedEBDigiCollection"));
  //EBDigiCollectionT_ = consumes<EBDigiCollection>(iConfig.getParameter<edm::InputTag>("EBDigiCollection"));
  EERecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("reducedEERecHitCollection"));
  //EERecHitCollectionT_ = consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("EERecHitCollection"));
  HBHERecHitCollectionT_ = consumes<HBHERecHitCollection>(iConfig.getParameter<edm::InputTag>("reducedHBHERecHitCollection"));
  TRKRecHitCollectionT_  = consumes<TrackingRecHitCollection>(iConfig.getParameter<edm::InputTag>("trackRecHitCollection"));

  genParticleCollectionT_ = consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("genParticleCollection"));
  photonCollectionT_ = consumes<reco::PhotonCollection>(iConfig.getParameter<edm::InputTag>("gedPhotonCollection"));
  jetCollectionT_ = consumes<reco::PFJetCollection>(iConfig.getParameter<edm::InputTag>("ak4PFJetCollection"));
  genJetCollectionT_ = consumes<reco::GenJetCollection>(iConfig.getParameter<edm::InputTag>("genJetCollection"));
  trackCollectionT_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("trackCollection"));
  pfCollectionT_ = consumes<PFCollection>(iConfig.getParameter<edm::InputTag>("pfCollection"));


  //johnda add configuration
  minJetPt_ = iConfig.getParameter<double>("minJetPt");
  maxJetEta_ = iConfig.getParameter<double>("maxJetEta");
  mode_ = iConfig.getParameter<std::string>("mode");
  std::cout << " Mode set to " << mode_ << std::endl;
  if( mode_ == "JetLevel"){
    doJets_    = true;
    nJets_ = iConfig.getParameter<int>("nJets");
    std::cout << " \t nJets set to " << nJets_ << std::endl;
  }else if (mode_ == "EventLevel"){
    doJets_ = false;
  } else {
    std::cout << " Assuming EventLevel Config. " << std::endl;
    doJets_ = false;
  }

  // Initialize file writer
  // NOTE: initializing dynamic-memory histograms outside of TFileService
  // will cause memory leaks
  usesResource("TFileService");
  edm::Service<TFileService> fs;
  h_sel = fs->make<TH1F>("h_sel", "isSelected;isSelected;Events", 2, 0., 2.);

  //////////// TTree //////////

  // These will be use to create the actual images
  RHTree = fs->make<TTree>("RHTree", "RecHit tree");
  if(doJets_){
    branchesEvtSel_jet( RHTree, fs );
  }else{
    branchesEvtSel( RHTree, fs );
  }

  branchesEB           ( RHTree, fs );
  branchesEE           ( RHTree, fs );
  branchesHBHE         ( RHTree, fs );
  branchesECALatHCAL   ( RHTree, fs );
  branchesECALstitched ( RHTree, fs );
  branchesHCALatEBEE   ( RHTree, fs );
  branchesTracksAtEBEE(RHTree, fs);
  branchesTracksAtECALstitched( RHTree, fs);
  branchesPFCandsAtEBEE(RHTree, fs);
  branchesPFCandsAtECALstitched( RHTree, fs);

  //branchesTRKlayersAtEBEE(RHTree, fs);
  //branchesTRKlayersAtECAL(RHTree, fs);
  //branchesTRKvolumeAtEBEE(RHTree, fs);
  //branchesTRKvolumeAtECAL(RHTree, fs);

  // For FC inputs
  //RHTree->Branch("FC_inputs",      &vFC_inputs_);

} // constructor
//
RecHitAnalyzer::~RecHitAnalyzer()
{

  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)

}
//
// member functions
//
// ------------ method called for each event  ------------
void
RecHitAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  using namespace edm;

  // ----- Apply event selection cuts ----- //

  bool passedSelection = false;
  if(doJets_){
    passedSelection = runEvtSel_jet( iEvent, iSetup );
  }else{
    passedSelection = runEvtSel( iEvent, iSetup );
  }


  if ( !passedSelection ) return;

  fillEB( iEvent, iSetup );
  fillEE( iEvent, iSetup );
  fillHBHE( iEvent, iSetup );
  fillECALatHCAL( iEvent, iSetup );
  fillECALstitched( iEvent, iSetup );
  fillHCALatEBEE( iEvent, iSetup );
  fillTracksAtEBEE( iEvent, iSetup );
  fillTracksAtECALstitched( iEvent, iSetup );
  fillPFCandsAtEBEE( iEvent, iSetup );
  fillPFCandsAtECALstitched( iEvent, iSetup );
  //fillTRKlayersAtEBEE( iEvent, iSetup );
  //fillTRKlayersAtECAL( iEvent, iSetup );
  //fillTRKvolumeAtEBEE( iEvent, iSetup );
  //fillTRKvolumeAtECAL( iEvent, iSetup );

  ////////////// 4-Momenta //////////
  //fillFC( iEvent, iSetup );

  // Fill RHTree
  RHTree->Fill();
  h_sel->Fill( 1. );

} // analyze()


// ------------ method called once each job just before starting event loop  ------------
void 
RecHitAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
RecHitAnalyzer::endJob() 
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
RecHitAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

const reco::PFCandidate*
RecHitAnalyzer::getPFCand(edm::Handle<PFCollection> pfCands, float eta, float phi, float& minDR, bool debug ) {

  float minDR2 = 100;
  const reco::PFCandidate* minDRCand = nullptr;
  
  for ( PFCollection::const_iterator iPFC = pfCands->begin();
        iPFC != pfCands->end(); ++iPFC ) {
    //if((iPFC->particleId() != 1) && (iPFC->particleId() != 2) && (iPFC->particleId() != 3) ) continue;
    //reco::TrackRef thisTrk = iPFC->trackRef();
    const reco::Track* thisTrk = iPFC->bestTrack();
    if(!thisTrk) continue;
    //if(!thisTrk.isNonnull()) continue;
    //if(!thisTrk.isAvailable()) continue;

    float etaDiff = (eta - thisTrk->eta());
    float phiDiff = (phi - thisTrk->phi());
    if(phiDiff > 3.14)  phiDiff -= 2*3.14;
    if(phiDiff < -3.14)  phiDiff += 2*3.14;
    if(debug) std::cout << "\t\tphidiff " << phiDiff << std::endl;
    float thisDr2 = (etaDiff*etaDiff + phiDiff*phiDiff);
    if(debug) std::cout << "\tthisDr2: " << thisDr2 << " " << thisTrk->pt() << " " << iPFC->particleId() << std::endl;

    const reco::PFCandidate& thisPFCand = (*iPFC);
      
    if( (thisDr2 < 0.001) && (thisDr2 <minDR2)){
      minDR2    = thisDr2; 
      minDRCand = &thisPFCand;
    }
  }

  minDR = sqrtf(minDR2);
  return minDRCand;  
}


const reco::Track*
RecHitAnalyzer::getTrackCand(edm::Handle<reco::TrackCollection> trackCands, float eta, float phi, float& minDR, bool debug ) {

  float minDR2 = 100;
  const reco::Track* minDRCand = nullptr;
  reco::Track::TrackQuality tkQt_ = reco::Track::qualityByName("highPurity");

  for ( reco::TrackCollection::const_iterator iTk = trackCands->begin();
        iTk != trackCands->end(); ++iTk ) {
    if ( !(iTk->quality(tkQt_)) ) continue;  


    float etaDiff = (eta - iTk->eta());
    float phiDiff = (phi - iTk->phi());
    if(phiDiff > 3.14)  phiDiff -= 2*3.14;
    if(phiDiff < -3.14)  phiDiff += 2*3.14;
    if(debug) std::cout << "\t\tphidiff " << phiDiff << std::endl;
    float thisDr2 = (etaDiff*etaDiff + phiDiff*phiDiff);
    if(debug) std::cout << "\tthisDr2: " << thisDr2 << " " << iTk->pt() << std::endl;

    const reco::Track& thisTrackCand = (*iTk);
      
    if( (thisDr2 < 0.001) && (thisDr2 <minDR2)){
      minDR2    = thisDr2; 
      minDRCand = &thisTrackCand;
    }
  }

  minDR = sqrtf(minDR2);
  return minDRCand;  
}



/*
//____ Fill FC diphoton variables _____//
void RecHitAnalyzer::fillFC ( const edm::Event& iEvent, const edm::EventSetup& iSetup ) {

  edm::Handle<reco::PhotonCollection> photons;
  iEvent.getByToken(photonCollectionT_, photons);

  vFC_inputs_.clear();

  int ptOrder[2] = {0, 1};
  if ( vPho_[1].Pt() > vPho_[0].Pt() ) {
      ptOrder[0] = 1;
      ptOrder[1] = 0;
  }
  for ( int i = 0; i < 2; i++ ) {
    vFC_inputs_.push_back( vPho_[ptOrder[i]].Pt()/m0_ );
    vFC_inputs_.push_back( vPho_[ptOrder[i]].Eta() );
  }
  vFC_inputs_.push_back( TMath::Cos(vPho_[0].Phi()-vPho_[1].Phi()) );

} // fillFC() 
*/

//define this as a plug-in
DEFINE_FWK_MODULE(RecHitAnalyzer);
