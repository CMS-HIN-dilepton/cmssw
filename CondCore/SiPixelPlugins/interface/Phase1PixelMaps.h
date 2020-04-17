#ifndef CONDCORE_SIPIXELPLUGINS_PHASE1PIXELMAPS_H
#define CONDCORE_SIPIXELPLUGINS_PHASE1PIXELMAPS_H

#include "TH2Poly.h"
#include "CalibTracker/StandaloneTrackerTopology/interface/StandaloneTrackerTopology.h"
#include "CondCore/SiPixelPlugins/interface/SiPixelPayloadInspectorHelper.h"

/*--------------------------------------------------------------------
/ Ancillary class to build pixel phase-1 tracker maps
/--------------------------------------------------------------------*/
class Phase1PixelMaps {
public: 
  Phase1PixelMaps(const char* option) :
    m_option{option},
    m_trackerTopo{StandaloneTrackerTopology::fromTrackerParametersXMLFile(edm::FileInPath("Geometry/TrackerCommonData/data/PhaseI/trackerParameters.xml").fullPath())}
  {
    // store the file in path for the corners (BPIX)
    for (unsigned int i = 1; i <= 4; i++) {
      m_cornersBPIX.push_back(edm::FileInPath(Form("DQM/SiStripMonitorClient/data/Geometry/vertices_barrel_%i", i)));
    }
    
    // store the file in path for the corners (BPIX)
    for (int j : {-3, -2, -1, 1, 2, 3}) {
      m_cornersFPIX.push_back(edm::FileInPath(Form("DQM/SiStripMonitorClient/data/Geometry/vertices_forward_%i", j)));
    }
  }
  
  ~Phase1PixelMaps(){}
  
  //============================================================================
  void bookBarrelHistograms(const std::string& currentHistoName) {
    std::string histName;
    TH2Poly* th2p;
    
    for (unsigned i = 0; i < 4; ++i) {
      histName = "barrel_layer_";
      
      th2p = new TH2Poly(
			 (histName + std::to_string(i + 1)).c_str(), Form("PXBMap - Layer %i", i + 1), -15.0, 15.0, 0.0, 5.0);
      
      th2p->SetFloat();
      
      th2p->GetXaxis()->SetTitle("z [cm]");
      th2p->GetYaxis()->SetTitle("ladder");
      th2p->SetStats(false);
      th2p->SetOption(m_option);
      th2PolyBarrel[currentHistoName].push_back(th2p);
    }
    
    th2p = new TH2Poly("barrel_summary", "PXBMap", -5.0, 5.0, 0.0, 5.0);
    th2p->SetFloat();
    
    th2p->GetXaxis()->SetTitle("");
    th2p->GetYaxis()->SetTitle("~ladder");
    th2p->SetStats(false);
    th2p->SetOption(m_option);
    th2PolyBarrelSummary[currentHistoName] = th2p;
  }
  
  //============================================================================
  void bookForwardHistograms(const std::string& currentHistoName) {
    std::string histName;
    TH2Poly* th2p;
    
    for (unsigned side = 1; side <= 2; ++side) {
      for (unsigned disk = 1; disk <= 3; ++disk) {
	histName = "forward_disk_";
	
	th2p = new TH2Poly((histName + std::to_string((side == 1 ? -(int(disk)) : (int)disk))).c_str(),
			   Form("PXFMap - Side %i Disk %i", side, disk),
			   -15.0,
			   15.0,
			   -15.0,
			   15.0);
	th2p->SetFloat();
	th2p->GetXaxis()->SetTitle("x [cm]");
	th2p->GetYaxis()->SetTitle("y [cm]");
	th2p->SetStats(false);
	th2p->SetOption(m_option);
	pxfTh2PolyForward[currentHistoName].push_back(th2p);
      }
    }

    th2p = new TH2Poly("forward_summary", "PXFMap", -40.0, 50.0, -20.0, 90.0);
    th2p->SetFloat();
    
    th2p->GetXaxis()->SetTitle("");
    th2p->GetYaxis()->SetTitle("");
    th2p->SetStats(false);
    th2p->SetOption(m_option);
    pxfTh2PolyForwardSummary[currentHistoName] = th2p;
  }
  
  //============================================================================
  void bookBarrelBins(const std::string& currentHistoName) {
    
    auto theIndexedCorners = SiPixelPI::retrieveCorners(m_cornersBPIX, 4);
    
    for (const auto& entry : theIndexedCorners) {
      auto id = entry.first;
      auto detid = DetId(id);
      if (detid.subdetId() != PixelSubdetector::PixelBarrel)
	continue;
      
      int layer = m_trackerTopo.pxbLayer(detid);
      int ladder = m_trackerTopo.pxbLadder(detid);
      
      auto theVectX = entry.second.first;
      auto theVectY = entry.second.second;
      
      float vertX[] = {theVectX[0], theVectX[1], theVectX[2], theVectX[3], theVectX[4]};
      float vertY[] = {(ladder - 1.0f), (ladder - 1.0f), (float)ladder, (float)ladder, (ladder - 1.0f)};
      
      bins[id] = new TGraph(5, vertX, vertY);
      bins[id]->SetName(TString::Format("%u", id));

      // Summary plot
      for (unsigned k = 0; k < 5; ++k) {
	vertX[k] += ((layer == 2 || layer == 3) ? 0.0f : -60.0f);
	vertY[k] += ((layer > 2) ? 30.0f : 0.0f);
      }
      
      binsSummary[id] = new TGraph(5, vertX, vertY);
      binsSummary[id]->SetName(TString::Format("%u", id));
      
      th2PolyBarrel[currentHistoName][layer - 1]->AddBin(bins[id]->Clone());
      th2PolyBarrelSummary[currentHistoName]->AddBin(binsSummary[id]->Clone());
    }
  }
  
  //============================================================================
  void bookForwardBins(const std::string& currentHistoName) {
    
    auto theIndexedCorners = SiPixelPI::retrieveCorners(m_cornersFPIX, 3);
    
    for (const auto& entry : theIndexedCorners) {
      auto id = entry.first;
      auto detid = DetId(id);
      if (detid.subdetId() != PixelSubdetector::PixelEndcap)
	continue;
      
      int disk = m_trackerTopo.pxfDisk(detid);
      int side = m_trackerTopo.pxfSide(detid);
      
      unsigned mapIdx = disk + (side - 1) * 3 - 1;
      
      auto theVectX = entry.second.first;
      auto theVectY = entry.second.second;
      
      float vertX[] = {theVectX[0], theVectX[1], theVectX[2], theVectX[3]};
      float vertY[] = {theVectY[0], theVectY[1], theVectY[2], theVectY[3]};

      bins[id] = new TGraph(4, vertX, vertY);
      bins[id]->SetName(TString::Format("%u", id));

      // Summary plot
      for (unsigned k = 0; k < 4; ++k) {
	vertX[k] += (float(side) - 1.5f) * 40.0f;
	vertY[k] += (disk - 1) * 35.0f;
      }
      
      binsSummary[id] = new TGraph(4, vertX, vertY);
      binsSummary[id]->SetName(TString::Format("%u", id));
      
      pxfTh2PolyForward[currentHistoName][mapIdx]->AddBin(bins[id]->Clone());
      pxfTh2PolyForwardSummary[currentHistoName]->AddBin(binsSummary[id]->Clone());
    }
  }
  
  //============================================================================
  template <typename type>
  void fillBarrelBin(const std::string& currentHistoName,
		     unsigned int id,
		     type value){
    int layer = m_trackerTopo.pxbLayer(id);
    th2PolyBarrel[currentHistoName][layer - 1]->Fill(TString::Format("%u", id), value);
  }
  
  //============================================================================
  template <typename type>
  void fillEndcapBin(const std::string& currentHistoName,
		     unsigned int id,
		     type value){
    int disk = m_trackerTopo.pxfDisk(id);
    int side = m_trackerTopo.pxfSide(id);
    unsigned mapIdx = disk + (side - 1) * 3 - 1;
    pxfTh2PolyForward[currentHistoName][mapIdx]->Fill(TString::Format("%u", id), value);
  }
  
  //============================================================================
  void DrawBarrelMaps(const std::string& currentHistoName,
		      TCanvas &canvas){
    canvas.Divide(2, 2);
    for (int i = 1; i <= 4; i++) {
      canvas.cd(i);
      if( strcmp(m_option, "text") == 0){
	th2PolyBarrel[currentHistoName].at(i-1)->SetMarkerColor(kRed);
      }
      th2PolyBarrel[currentHistoName].at(i-1)->Draw();
    }
  }
  
  //============================================================================
  void DrawEndcapMaps(const std::string& currentHistoName,
		      TCanvas &canvas){
    canvas.Divide(3, 2);
    for (int i = 1; i <= 6; i++) {
      canvas.cd(i);
      if( strcmp(m_option, "text") == 0){
	pxfTh2PolyForward[currentHistoName].at(i-1)->SetMarkerColor(kRed);
      }
      pxfTh2PolyForward[currentHistoName].at(i-1)->Draw();
    }
  }

private:
  Option_t* m_option;
  TrackerTopology m_trackerTopo;
  
  std::map<uint32_t, TGraph*> bins, binsSummary;
  std::map<std::string, std::vector<TH2Poly*>> th2PolyBarrel;
  std::map<std::string, TH2Poly*> th2PolyBarrelSummary;
  std::map<std::string, std::vector<TH2Poly*>> pxfTh2PolyForward;
  std::map<std::string, TH2Poly*> pxfTh2PolyForwardSummary;
  
  std::vector<edm::FileInPath> m_cornersBPIX;
  std::vector<edm::FileInPath> m_cornersFPIX;
};


#endif
