//
// Package:         RecoTracker/RoadSearchSeedFinder
// Class:           RoadSearchSeedFinderAlgorithm
// 
// Description:     Loops over Roads, checks for every
//                  RoadSeed if hits are in the inner and
//                  outer SeedRing, applies cuts for all 
//                  combinations of inner and outer SeedHits,
//                  stores valid combination in TrajectorySeed
//
// Original Author: Oliver Gutsche, gutsche@fnal.gov
// Created:         Sat Jan 14 22:00:00 UTC 2006
//
// $Author: tboccali $
// $Date: 2006/07/24 19:44:42 $
// $Revision: 1.11 $
//

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "RecoTracker/RoadSearchSeedFinder/interface/RoadSearchSeedFinderAlgorithm.h"

#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripMatchedRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2D.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrajectoryState/interface/PTrajectoryStateOnDet.h"
#include "DataFormats/TrajectoryState/interface/LocalTrajectoryParameters.h"

#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "RecoTracker/RoadMapRecord/interface/Roads.h"
#include "RecoTracker/TkTrackingRegions/interface/GlobalTrackingRegion.h"
#include "RecoTracker/TkSeedGenerator/interface/FastHelix.h"

#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/GeomPropagators/interface/AnalyticalPropagator.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

RoadSearchSeedFinderAlgorithm::RoadSearchSeedFinderAlgorithm(const edm::ParameterSet& conf) : conf_(conf) { 
}

RoadSearchSeedFinderAlgorithm::~RoadSearchSeedFinderAlgorithm() {
}


void RoadSearchSeedFinderAlgorithm::run(const edm::Handle<SiStripMatchedRecHit2DCollection> &handle,
                                        const edm::Handle<SiStripRecHit2DCollection> &handle2,
			      const edm::EventSetup& es,
			      TrajectorySeedCollection &output)
{

  const SiStripMatchedRecHit2DCollection* input = handle.product();
  const SiStripRecHit2DCollection* input2 = handle2.product();

  const std::vector<DetId> availableIDs = input->ids();
  const std::vector<DetId> availableIDs2 = input2->ids();

  // get roads
  edm::ESHandle<Roads> roads;
  es.get<TrackerDigiGeometryRecord>().get(roads);

  // calculate maximal possible delta phi for given delta r and parameter pTmin
  double ptmin = conf_.getParameter<double>("MinimalReconstructedTransverseMomentum");

  // correction for B given in T, delta r given in cm, ptmin given in GeV/c
  double speedOfLight = 2.99792458e8;
  double unitCorrection = speedOfLight * 1e-2 * 1e-9;

  // B in T, right now hardcoded, has to come from magnetic field service
  double B = 4.0;

  // loop over seed Ring pairs
  for ( Roads::const_iterator road = roads->begin(); road != roads->end(); ++road ) {

    Roads::RoadSeed seed = (*road).first;

    // loop over detid's in seed rings
    for ( Ring::const_iterator innerRingDetId = seed.first.begin(); innerRingDetId != seed.first.end(); ++innerRingDetId ) {

      if ( availableIDs.end() != std::find(availableIDs.begin(),availableIDs.end(),innerRingDetId->second) ) {

	for ( Ring::const_iterator outerRingDetId = seed.second.begin(); outerRingDetId != seed.second.end(); ++outerRingDetId ) {

	  if ( availableIDs2.end() != std::find(availableIDs2.begin(),availableIDs2.end(),outerRingDetId->second) ) {

	    SiStripMatchedRecHit2DCollection::range innerSeedDetHits = input->get(innerRingDetId->second);
      
	    // loop over inner dethits
	    for ( SiStripMatchedRecHit2DCollection::const_iterator innerSeedDetHit = innerSeedDetHits.first;
		  innerSeedDetHit != innerSeedDetHits.second; ++innerSeedDetHit ) {

	      SiStripRecHit2DCollection::range outerSeedDetHits = input2->get(outerRingDetId->second);

	      for ( SiStripRecHit2DCollection::const_iterator outerSeedDetHit = outerSeedDetHits.first;
		    outerSeedDetHit != outerSeedDetHits.second; ++outerSeedDetHit ) {

		// get tracker geometry
		edm::ESHandle<TrackerGeometry> tracker;
		es.get<TrackerDigiGeometryRecord>().get(tracker);

		GlobalPoint inner = tracker->idToDet(innerSeedDetHit->geographicalId())->surface().toGlobal(innerSeedDetHit->localPosition());
		GlobalPoint outer = tracker->idToDet(outerSeedDetHit->geographicalId())->surface().toGlobal(outerSeedDetHit->localPosition());

		// calculate deltaPhi in [0,2pi]
		double deltaPhi = std::abs(inner.phi() - outer.phi());
		double pi = 3.14159265358979312;
		if ( deltaPhi < 0 ) deltaPhi = 2*pi - deltaPhi;
	    
		double innerr = std::sqrt(inner.x()*inner.x()+inner.y()*inner.y());
		double outerr = std::sqrt(outer.x()*outer.x()+outer.y()*outer.y());

		// calculate maximal delta phi in [0,2pi]
		double deltaPhiMax = std::abs( std::asin(unitCorrection * B * innerr / ptmin) - std::asin(unitCorrection * B * outerr / ptmin) );
		if ( deltaPhiMax < 0 ) deltaPhiMax = 2*pi - deltaPhiMax;

		if ( deltaPhi <= deltaPhiMax ) {
	      
		  // use correct tk seed generator from consecutive hits
		  // make PTrajectoryOnState from two hits and region (beamspot)
		  GlobalTrackingRegion region;
		  GlobalError vtxerr( std::sqrt(region.originRBound()),
				      0, std::sqrt(region.originRBound()),
				      0, 0, std::sqrt(region.originZBound()));

		  double x0=0.0,y0=0.0,z0=0.0;
                  bool nofieldcosmic = conf_.getParameter<bool>("StraightLineNoBeamSpotSeed");
                  if (nofieldcosmic){
                    double phi0=atan2(outer.y()-inner.y(),outer.x()-inner.x());
                    double alpha=atan2(inner.y(),inner.x());
                    double d1=sqrt(inner.x()*inner.x()+inner.y()*inner.y());
                    double d0=-d1*sin(alpha-phi0); x0=d0*sin(phi0); y0=-d0*cos(phi0);
                    double l1=0.0,l2=0.0;
                    if (fabs(cos(phi0))>0.1){
                      l1=(inner.x()-x0)/cos(phi0);l2=(outer.x()-x0)/cos(phi0);  
                    }else{
                      l1=(inner.y()-y0)/sin(phi0);l2=(outer.y()-y0)/sin(phi0);  
                    }
                    z0=(l2*inner.z()-l1*outer.z())/(l2-l1);
//                    std::cout << "In RSSF, d0,phi0,x0,y0,z0 " << d0 << " " << phi0 << " " << x0 << " " << y0 << " " << z0 << std::endl;
                  }

		  FastHelix helix(outer, inner, GlobalPoint(x0,y0,z0),es);

                  FreeTrajectoryState fts( helix.stateAtVertex().parameters(),
                                           initialError( &(*outerSeedDetHit), &(*innerSeedDetHit),
                                                         region.origin(), vtxerr));

                  edm::ESHandle<MagneticField> pSetup;
                  es.get<IdealMagneticFieldRecord>().get(pSetup);

                  AnalyticalPropagator  thePropagator(&(*pSetup), alongMomentum);

                  KFUpdator     theUpdator;

                  const TrajectoryStateOnSurface  innerState = thePropagator.propagate(fts,tracker->idToDet(innerSeedDetHit->geographicalId())->surface());

		  if (innerState.isValid()){
		    //
		    // create the OwnVector of TrackingRecHits
		    //
		    edm::OwnVector<TrackingRecHit> rh;

		    //
		    // memory leak??? TB
		    //
		    rh.push_back(innerSeedDetHit->clone());
		    rh.push_back(outerSeedDetHit->clone());
		    TrajectoryStateTransform transformer;
		    
		    PTrajectoryStateOnDet * PTraj=  transformer.persistentState(innerState, innerSeedDetHit->geographicalId().rawId());
		    TrajectorySeed ts(*PTraj,rh,alongMomentum);
		    
                    // 060811/OLI: memory leak fix as suggested by Chris
                    delete PTraj;
		    
                    // add seed to collection
		    output.push_back(ts);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  edm::LogInfo("RoadSearch") << "Found " << output.size() << " seeds."; 

};

CurvilinearTrajectoryError RoadSearchSeedFinderAlgorithm::
initialError( const TrackingRecHit* outerHit,
              const TrackingRecHit* innerHit,
              const GlobalPoint& vertexPos,
              const GlobalError& vertexErr)
{
  AlgebraicSymMatrix C(5,1);

  float zErr = vertexErr.czz();
  float transverseErr = vertexErr.cxx(); // assume equal cxx cyy 
  C[3][3] = transverseErr;
  C[4][4] = zErr;

  return CurvilinearTrajectoryError(C);
}
