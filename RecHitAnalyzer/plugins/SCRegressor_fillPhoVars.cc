#include "MLAnalyzer/RecHitAnalyzer/interface/SCRegressor.h"

// Initialize branches _____________________________________________________//
void SCRegressor::branchesPhoVars ( TTree* tree, edm::Service<TFileService> &fs )
{

  tree->Branch("pho_pT",             &vPho_pT_);
  tree->Branch("pho_E",              &vPho_E_);
  tree->Branch("pho_eta",            &vPho_eta_);
  tree->Branch("pho_phi",            &vPho_phi_);
  tree->Branch("pho_ecalEPostCorr",  &vPho_ecalEPostCorr_);

  tree->Branch("pho_r9",             &vPho_r9_);
  tree->Branch("pho_sieie",          &vPho_sieie_);
  tree->Branch("pho_phoIso",         &vPho_phoIso_);
  tree->Branch("pho_chgIso",         &vPho_chgIso_);
  tree->Branch("pho_chgIsoWrongVtx", &vPho_chgIsoWrongVtx_);
  tree->Branch("pho_Eraw",           &vPho_Eraw_);
  tree->Branch("pho_phiWidth",       &vPho_phiWidth_);
  tree->Branch("pho_etaWidth",       &vPho_etaWidth_);
  tree->Branch("pho_scEta",          &vPho_scEta_);
  tree->Branch("pho_sieip",          &vPho_sieip_);
  tree->Branch("pho_s4",             &vPho_s4_);
  tree->Branch("pho_rho",            &vPho_rho_);

  tree->Branch("pho_neuIso",         &vPho_neuIso_);
  tree->Branch("pho_ecalIso",        &vPho_ecalIso_);
  tree->Branch("pho_trkIso",         &vPho_trkIso_);
  tree->Branch("pho_hasPxlSeed",     &vPho_hasPxlSeed_);
  tree->Branch("pho_passEleVeto",    &vPho_passEleVeto_);
  tree->Branch("pho_HoE",            &vPho_HoE_);
  tree->Branch("pho_phoIsoCorr",     &vPho_phoIsoCorr_);
  tree->Branch("pho_ecalIsoCorr",    &vPho_ecalIsoCorr_);

  tree->Branch("pho_neuIsoCorr",     &vPho_neuIsoCorr_);
  tree->Branch("pho_chgIsoCorr",     &vPho_chgIsoCorr_);
  tree->Branch("pho_bdt",            &vPho_bdt_);
  tree->Branch("pho_idx",            &vPho_idx_);

  tree->Branch("evtPU",              &evtPU_);

} // branchesPhoVars()

// Fill PhoVars rechits _________________________________________________________________//
void SCRegressor::fillPhoVars ( const edm::Event& iEvent, const edm::EventSetup& iSetup )
{

  EcalClusterLazyTools clusterTools ( iEvent, iSetup, EBRecHitCollectionT_, EERecHitCollectionT_, ESRecHitCollectionT_);

  edm::Handle<double> rhoH;
  iEvent.getByToken( rhoLabel_, rhoH );
  float rho = *rhoH;
  //std::cout << "rho:" << *rhoH << std::endl;
  //std::cout << "rho:" << *(rhoH.product()) << std::endl;

  edm::Handle<std::vector<PileupSummaryInfo> > genPileupH;
  iEvent.getByToken(puCollectionT_, genPileupH);

  evtPU_ = -1.;
  if ( genPileupH.isValid() ) {
    for ( std::vector<PileupSummaryInfo>::const_iterator pu = genPileupH->begin(); pu != genPileupH->end(); ++pu ) {
      if ( pu->getBunchCrossing() != 0 ) continue;
      evtPU_ = pu->getTrueNumInteractions();
    }
  } // genPileupH.isValid()
  //std::cout << " evtPU:" << evtPU_ << std::endl;

  edm::Handle<PhotonCollection> photons;
  iEvent.getByToken(photonCollectionT_, photons);

  ////////// Store kinematics //////////

  vPho_pT_.clear();
  vPho_E_.clear();
  vPho_eta_.clear();
  vPho_phi_.clear();
  vPho_ecalEPostCorr_.clear();
  for ( int iP : vRegressPhoIdxs_ ) {

    PhotonRef iPho( photons, iP );
    // Fill branch arrays
    vPho_pT_.push_back( iPho->pt() );
    vPho_E_.push_back( iPho->energy() );
    vPho_eta_.push_back( iPho->eta() );
    vPho_phi_.push_back( iPho->phi() );
    //vPho_ecalEPostCorr_.push_back( iPho->userFloat("ecalEnergyPostCorr") ); //2017 only: https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPostRecoRecipes#Running_on_2017_MiniAOD_V2
    //std::cout << ">> PRESEL pho["<<iP<<"]: pt:" << iPho->pt() << " eta:" << iPho->eta() << " phi:" << iPho->phi() << std::endl;
  } // photons

  vPho_r9_.clear();
  vPho_sieie_.clear();
  vPho_phoIso_.clear();
  vPho_chgIso_.clear();
  vPho_chgIsoWrongVtx_.clear();
  vPho_Eraw_.clear();
  vPho_phiWidth_.clear();
  vPho_etaWidth_.clear();
  vPho_scEta_.clear();
  vPho_sieip_.clear();
  vPho_s4_.clear();
  vPho_rho_.clear();

  vPho_neuIso_.clear();
  vPho_ecalIso_.clear();
  vPho_trkIso_.clear();
  vPho_hasPxlSeed_.clear();
  vPho_passEleVeto_.clear();
  vPho_HoE_.clear();
  vPho_phoIsoCorr_.clear();
  vPho_ecalIsoCorr_.clear();

  vPho_neuIsoCorr_.clear();
  vPho_chgIsoCorr_.clear();
  vPho_bdt_.clear();
  vPho_idx_.clear();

  for ( int iP : vRegressPhoIdxs_ ) {

    PhotonRef iPho( photons, iP );
    reco::SuperClusterRef const& iSC = iPho->superCluster();
    std::vector<float> vCov = clusterTools.localCovariances( *(iSC->seed()) );

    vPho_r9_.push_back(             iPho->full5x5_r9() );
    vPho_sieie_.push_back(          iPho->full5x5_sigmaIetaIeta() );
    vPho_Eraw_.push_back(           iSC->rawEnergy() );
    vPho_phiWidth_.push_back(       iSC->phiWidth() );
    vPho_etaWidth_.push_back(       iSC->etaWidth() );
    vPho_scEta_.push_back(          iSC->eta() );
    vPho_sieip_.push_back(          vCov[1] );
    vPho_s4_.push_back(             clusterTools.e2x2( *(iSC->seed()) ) / clusterTools.e5x5( *(iSC->seed()) ) );
    vPho_rho_.push_back(            rho );

    vPho_trkIso_.push_back(         iPho->trkSumPtHollowConeDR03() );
    vPho_hasPxlSeed_.push_back(     iPho->hasPixelSeed() );
    vPho_HoE_.push_back(            iPho->hadTowOverEm() );

    /*
    vPho_phoIso_.push_back(         iPho->photonIso() );
    vPho_chgIso_.push_back(         iPho->chargedHadronIso() );
    vPho_chgIsoWrongVtx_.push_back( iPho->chargedHadronIsoWrongVtx() );
    */
    ///*
    // Only valid for ECAL barrel
    float EAPho = iPho->eta() < 1.0 ? 0.1113 : 0.0953;
    float EAChg = iPho->eta() < 1.0 ? 0.0112 : 0.0108;
    float EANeu = iPho->eta() < 1.0 ? 0.0668 : 0.1054;
    vPho_phoIsoCorr_.push_back(     std::max(iPho->userFloat("phoPhotonIsolation") - rho*EAPho, (float)0.) );
    vPho_ecalIsoCorr_.push_back(    std::max(iPho->ecalPFClusterIso() - rho*EAPho, (float)0.) );
    vPho_phoIso_.push_back(         iPho->userFloat("phoPhotonIsolation") );
    vPho_chgIso_.push_back(         iPho->userFloat("phoChargedIsolation") );
    vPho_chgIsoWrongVtx_.push_back( iPho->userFloat("phoWorstChargedIsolation") );
    vPho_neuIso_.push_back(         iPho->userFloat("phoNeutralHadronIsolation") );
    vPho_ecalIso_.push_back(        iPho->ecalPFClusterIso() );
    vPho_passEleVeto_.push_back(    iPho->passElectronVeto() );

    vPho_neuIsoCorr_.push_back(     std::max(iPho->userFloat("phoNeutralHadronIsolation") - rho*EANeu, (float)0.) );
    vPho_chgIsoCorr_.push_back(     std::max(iPho->userFloat("phoChargedIsolation") - rho*EAChg, (float)0.) );
    //vPho_bdt_.push_back(            iPho->userFloat("PhotonMVAEstimatorRunIIFall17v2Values")); // need to run EGamma post-reco tools
    vPho_bdt_.push_back(            iPho->userFloat("PhotonMVAEstimatorRunIIFall17v1p1Values"));
    vPho_idx_.push_back( iP );
    //*/

    /*
    std::cout << "HoE:" << iPho->hadTowOverEm() << std::endl;
    std::cout << "ChgIso:" << iPho->userFloat("phoChargedIsolation") << std::endl;
    //std::cout << "ChgIsoOld:" << iPho->chargedHadronIso() <<std::endl;
    std::cout << "PhoIso:" << iPho->userFloat("phoPhotonIsolation") << std::endl;
    //std::cout << "PhoIsoOld:" << iPho->photonIso() <<std::endl;
    std::cout << "NeuIso:" << iPho->userFloat("phoNeutralHadronIsolation") << std::endl;
    std::cout << "ChWorstIso:" << iPho->userFloat("phoWorstChargedIsolation") << std::endl;

    std::cout << "TrkIso:" << iPho->trackIso() << std::endl;
    std::cout << "trkSumPtHollowConeDR03:" << iPho->trkSumPtHollowConeDR03() << std::endl;
    std::cout << "ecalPFClusterIso:" << iPho->ecalPFClusterIso() << std::endl;
    std::cout << "ecalPFClusterIso, rho corr:" << std::max(iPho->ecalPFClusterIso() - rho*EAPho, (float)0.) << std::endl;

    std::cout << "pt:" << iPho->pt() << std::endl;
    std::cout << "E:" << iPho->energy() << std::endl;

    std::cout << "r9:"             << iPho->full5x5_r9() << std::endl;
    std::cout << "sieie:"          << iPho->full5x5_sigmaIetaIeta() << std::endl;
    std::cout << "phoIso:"         << iPho->photonIso() << std::endl;
    std::cout << "chgIso:"         << iPho->chargedHadronIso() << std::endl;
    std::cout << "chgIsoWrongVtx:" << iPho->chargedHadronIsoWrongVtx() << std::endl;

    std::cout << "Eraw:"           << iSC->rawEnergy() << std::endl;
    std::cout << "phiwidth:"       << iSC->phiWidth() << std::endl;
    std::cout << "etawidth:"       << iSC->etaWidth() << std::endl;
    std::cout << "eta:"            << iSC->eta() << std::endl;

    std::cout << "sieip:"          << vCov[1] << std::endl;
    std::cout << "s4:"             << clusterTools.e2x2( *(iSC->seed()) ) / clusterTools.e5x5( *(iSC->seed()) ) << std::endl;
    */
  } // photons

} // fillPhoVars()
