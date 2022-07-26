#include "MLAnalyzer/RecHitAnalyzer/interface/RecHitAnalyzer.h"
#include "DataFormats/BTauReco/interface/JetTag.h"

using std::vector;
using std::cout;
using std::endl;

TH1D *h_taujet_jet_pT;
TH1D *h_taujet_jet_E;
TH1D *h_taujet_jet_eta;
TH1D *h_taujet_jet_m0;
TH1D *h_taujet_jet_nJet;
vector<float> vTaujet_jet_pT_;
vector<float> vTaujet_jet_m0_;
vector<float> vTaujet_jet_eta_;
vector<float> vTaujet_jet_phi_;
vector<float> vTaujet_jet_truthLabel_;
vector<float> vTaujet_jet_truthDM_;
vector<float> vTaujet_jet_neutral_pT_;
vector<float> vTaujet_jet_neutral_m0_;
vector<float> vTaujet_jet_neutral_eta_;
vector<float> vTaujet_jet_neutral_phi_;
vector<vector<float>> vTaujet_jet_charged_indv_pt_;
vector<vector<float>> vTaujet_jet_neutral_indv_pt_;
vector<vector<float>> vTaujet_jet_charged_indv_eta_;
vector<vector<float>> vTaujet_jet_neutral_indv_eta_;
vector<vector<float>> vTaujet_jet_charged_indv_phi_;
vector<vector<float>> vTaujet_jet_neutral_indv_phi_;


// Initialize branches _____________________________________________________//
void RecHitAnalyzer::branchesEvtSel_jet_taujet( TTree* tree, edm::Service<TFileService> &fs ) {

  h_taujet_jet_pT    = fs->make<TH1D>("h_jet_pT"  , "p_{T};p_{T};Particles", 100,  0., 500.);
  h_taujet_jet_E     = fs->make<TH1D>("h_jet_E"   , "E;E;Particles"        , 100,  0., 800.);
  h_taujet_jet_eta   = fs->make<TH1D>("h_jet_eta" , "#eta;#eta;Particles"  , 100, -5., 5.);
  h_taujet_jet_nJet  = fs->make<TH1D>("h_jet_nJet", "nJet;nJet;Events"     ,  10,  0., 10.);
  h_taujet_jet_m0    = fs->make<TH1D>("h_jet_m0"  , "m0;m0;Events"         , 100,  0., 100.);

  tree->Branch("jetPt",          &vTaujet_jet_pT_);
  tree->Branch("jetM",           &vTaujet_jet_m0_);
  tree->Branch("jetEta",         &vTaujet_jet_eta_);
  tree->Branch("jetPhi",         &vTaujet_jet_phi_);
  tree->Branch("jet_truthLabel", &vTaujet_jet_truthLabel_);

  tree->Branch("jet_truthDM", &vTaujet_jet_truthDM_);
  tree->Branch("neutralPt", &vTaujet_jet_neutral_pT_);
  tree->Branch("neutralM", &vTaujet_jet_neutral_m0_);
  tree->Branch("neutralEta", &vTaujet_jet_neutral_eta_);
  tree->Branch("neutralPhi", &vTaujet_jet_neutral_phi_);
  tree->Branch("jet_charged_indv_pt", &vTaujet_jet_charged_indv_pt_);
  tree->Branch("jet_neutral_indv_pt", &vTaujet_jet_neutral_indv_pt_);
  tree->Branch("jet_charged_indv_eta", &vTaujet_jet_charged_indv_eta_);
  tree->Branch("jet_neutral_indv_eta", &vTaujet_jet_neutral_indv_eta_);
  tree->Branch("jet_charged_indv_phi", &vTaujet_jet_charged_indv_phi_);
  tree->Branch("jet_neutral_indv_phi", &vTaujet_jet_neutral_indv_phi_);

} // branchesEvtSel_jet_taujet()

// Run jet selection _____________________________________________________//
bool RecHitAnalyzer::runEvtSel_jet_taujet( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{

  edm::Handle<reco::PFJetCollection> jets;
  iEvent.getByToken(jetCollectionT_, jets);

  vJetIdxs.clear();
  vTaujet_jet_pT_.clear();
  vTaujet_jet_m0_.clear();
  vTaujet_jet_eta_.clear();
  vTaujet_jet_phi_.clear();
  vTaujet_jet_truthLabel_.clear();
  vTaujet_jet_truthDM_.clear();
  vTaujet_jet_neutral_pT_.clear();
  vTaujet_jet_neutral_m0_.clear();
  vTaujet_jet_neutral_eta_.clear();
  vTaujet_jet_neutral_phi_.clear();
  vTaujet_jet_charged_indv_pt_.clear();
  vTaujet_jet_neutral_indv_pt_.clear();
  vTaujet_jet_charged_indv_eta_.clear();
  vTaujet_jet_neutral_indv_eta_.clear();
  vTaujet_jet_charged_indv_phi_.clear();
  vTaujet_jet_neutral_indv_phi_.clear();

  int nJet = 0;
  // Loop over jets
  for ( unsigned iJ(0); iJ != jets->size(); ++iJ ) {

    reco::PFJetRef iJet( jets, iJ );
    if ( std::abs(iJet->pt()) < minJetPt_ ) continue;
    if ( std::abs(iJet->eta()) > maxJetEta_) continue;
    if ( debug ) std::cout << " >> jet[" << iJ << "]Pt:" << iJet->pt() << " jetE:" << iJet->energy() << " jetM:" << iJet->mass() << std::endl;

    vJetIdxs.push_back(iJ);

    nJet++;
    if ( (nJets_ > 0) && (nJet >= nJets_) ) break;

  } // jets


  if ( debug ) {
    for(int thisJetIdx : vJetIdxs)
      std::cout << " >> vJetIdxs:" << thisJetIdx << std::endl;
  }

  if ( (nJets_ > 0) && (nJet != nJets_) ){
    if ( debug ) std::cout << " Fail jet multiplicity:  " << nJet << " != " << nJets_ << std::endl;
    return false;
  }

  if ( vJetIdxs.size() == 0){
    if ( debug ) std::cout << " No passing jets...  " << std::endl;
    return false;
  }

  if ( debug ) std::cout << " >> has_jet_taujet: passed" << std::endl;
  return true;

} // runEvtSel_jet_taujet() 

// Fill branches and histograms _____________________________________________________//
void RecHitAnalyzer::fillEvtSel_jet_taujet( const edm::Event& iEvent, const edm::EventSetup& iSetup ) {

  edm::Handle<reco::PFJetCollection> jets;
  iEvent.getByToken(jetCollectionT_, jets);

  edm::Handle<reco::GenParticleCollection> genParticles;
  iEvent.getByToken( genParticleCollectionT_, genParticles );

  edm::Handle<edm::View<reco::Jet> > recoJetCollection;
  iEvent.getByToken(recoJetsT_, recoJetCollection);

  edm::Handle<reco::JetTagCollection> btagDiscriminators;
  iEvent.getByToken(jetTagCollectionT_, btagDiscriminators);

  edm::Handle<std::vector<reco::CandIPTagInfo> > ipTagInfo;
  iEvent.getByToken(ipTagInfoCollectionT_, ipTagInfo);

  edm::Handle<reco::VertexCollection> vertexInfo;
  iEvent.getByToken(vertexCollectionT_, vertexInfo);
  const reco::VertexCollection& vtxs = *vertexInfo;
	      
 
  h_taujet_jet_nJet->Fill( vJetIdxs.size() );
  // Fill branches and histograms 
  for(int thisJetIdx : vJetIdxs){
    reco::PFJetRef thisJet( jets, thisJetIdx );
    if ( debug ) std::cout << " >> Jet[" << thisJetIdx << "] Pt:" << thisJet->pt() << std::endl;

    if (debug) std::cout << " Passed Jet " << " Pt:" << thisJet->pt()  << " Eta:" << thisJet->eta()  << " Phi:" << thisJet->phi() 
			 << " jetE:" << thisJet->energy() << " jetM:" << thisJet->mass() 
			 << " photonE:" << thisJet->photonEnergy()  
			 << " chargedHadronEnergy:" << thisJet->chargedHadronEnergy()  
			 << " neutralHadronEnergy :" << thisJet->neutralHadronEnergy()
			 << " electronEnergy	 :" << thisJet->electronEnergy	()
			 << " muonEnergy		 :" << thisJet->muonEnergy		()
			 << " HFHadronEnergy	 :" << thisJet->HFHadronEnergy	()
			 << " HFEMEnergy		 :" << thisJet->HFEMEnergy		()
			 << " chargedEmEnergy	 :" << thisJet->chargedEmEnergy	()
			 << " chargedMuEnergy	 :" << thisJet->chargedMuEnergy	()
			 << " neutralEmEnergy	 :" << thisJet->neutralEmEnergy	()
			 << std::endl;

    h_taujet_jet_pT->Fill( std::abs(thisJet->pt()) );
    h_taujet_jet_E->Fill( thisJet->energy() );
    h_taujet_jet_m0->Fill( thisJet->mass() );
    h_taujet_jet_eta->Fill( thisJet->eta() );
    vTaujet_jet_pT_.push_back( std::abs(thisJet->pt()) );
    vTaujet_jet_m0_.push_back( thisJet->mass() );
    vTaujet_jet_eta_.push_back( thisJet->eta() );
    vTaujet_jet_phi_.push_back( thisJet->phi() );

    std::pair<int, reco::GenTau*> match = getTruthLabelForTauJets(thisJet,genParticles,0.4, false);
    int truthLabel = match.first;
    vTaujet_jet_truthLabel_      .push_back(truthLabel);

    int truthDM=-1;
    float neutral_pT=0.;
    float neutral_M=0.;
    float neutral_eta=0.;
    float neutral_phi=0.;
    vector<float> charge_pt_indv;
    vector<float> neutral_pt_indv;
    vector<float> charge_eta_indv;
    vector<float> neutral_eta_indv;
    vector<float> charge_phi_indv;
    vector<float> neutral_phi_indv;

    // std::cout << truthLabel << std::endl;

    if (abs(truthLabel)==15) {
      // std::cout << "test" << std::endl; 
      truthDM = match.second->decay_mode();
      // Q: why are the neutral pT vars using the vis_p4?
      neutral_pT = match.second->neutral_p4().pt();
      neutral_M = match.second->neutral_p4().mass();
      neutral_eta = match.second->neutral_p4().eta();
      neutral_phi = match.second->neutral_p4().phi();
      // could store all Lorentz vectors instead?
      // std::cout<< "Storing the individual charged particle info" << std::endl;
      for (const auto &charged : match.second->charge_p4_indv()){
          charge_pt_indv.push_back(charged.pt());
          charge_eta_indv.push_back(charged.eta());
          charge_phi_indv.push_back(charged.phi());
        }
      // std::cout << "DEBUG: vector size" << match.second->neutral_p4_indv().size() << std::endl; 
      if (match.second->neutral_p4_indv().size()>0){
        // std::cout<< "Storing the individual neutral particle info (" << match.second->neutral_p4_indv().size() << ")" << std::endl;
        for (const auto &neutral : match.second->neutral_p4_indv()){
            neutral_pt_indv.push_back(neutral.pt());
            neutral_eta_indv.push_back(neutral.eta());
            neutral_phi_indv.push_back(neutral.phi());
          }
      } else{
        neutral_pt_indv.push_back(-1);
        neutral_eta_indv.push_back(-100); 
        neutral_phi_indv.push_back(-100);
        
      }
      
    } else{
      charge_pt_indv.push_back(-1);
      neutral_pt_indv.push_back(-1);
      charge_eta_indv.push_back(-100);
      neutral_eta_indv.push_back(-100);
      charge_phi_indv.push_back(-100);
      neutral_phi_indv.push_back(-100);
    }
    


    // std::cout << neutral_pT << std::endl;

    vTaujet_jet_truthDM_.push_back(truthDM);
    vTaujet_jet_neutral_pT_.push_back(neutral_pT);
    vTaujet_jet_neutral_m0_.push_back(neutral_M);
    vTaujet_jet_neutral_eta_.push_back(neutral_eta);
    vTaujet_jet_neutral_phi_.push_back(neutral_phi);
    // push back vector
    vTaujet_jet_charged_indv_pt_.push_back(charge_pt_indv);
    vTaujet_jet_neutral_indv_pt_.push_back(neutral_pt_indv);
    vTaujet_jet_charged_indv_eta_.push_back(charge_eta_indv);
    vTaujet_jet_neutral_indv_eta_.push_back(neutral_eta_indv);
    vTaujet_jet_charged_indv_phi_.push_back(charge_phi_indv);
    vTaujet_jet_neutral_indv_phi_.push_back(neutral_phi_indv);
  }//vJetIdxs


} // fillEvtSel_jet_taujet()
