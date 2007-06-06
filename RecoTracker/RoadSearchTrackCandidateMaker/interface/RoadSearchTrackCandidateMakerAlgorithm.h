#ifndef TrackCandidateMakerAlgorithm_h
#define TrackCandidateMakerAlgorithm_h

//
// Package:         RecoTracker/RoadSearchTrackCandidateMaker
// Class:           RoadSearchTrackCandidateMakerAlgorithm
// 
// Description:     Converts cleaned clouds into
//                  TrackCandidates using the 
//                  TrajectoryBuilder framework
//
// Original Author: Oliver Gutsche, gutsche@fnal.gov
// Created:         Wed Mar 15 13:00:00 UTC 2006
//
// $Author: gutsche $
// $Date: 2007/03/01 08:18:19 $
// $Revision: 1.7 $
//

#include <string>

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/Common/interface/OwnVector.h"
#include "DataFormats/RoadSearchCloud/interface/RoadSearchCloudCollection.h"
#include "DataFormats/TrackCandidate/interface/TrackCandidateCollection.h"
#include "RecoTracker/MeasurementDet/interface/MeasurementTracker.h"

#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "TrackingTools/Records/interface/TransientRecHitRecord.h" 
#include "TrackingTools/TransientTrackingRecHit/interface/TransientTrackingRecHit.h"
#include "TrackingTools/TransientTrackingRecHit/interface/TransientTrackingRecHitBuilder.h"
#include "TrackingTools/PatternTools/interface/TrajectoryMeasurement.h"

#include "RecoTracker/RoadSearchTrackCandidateMaker/interface/RoadSearchPairLess.h"

class TrajectoryStateUpdator;
class MeasurementEstimator;
class PropagatorWithMaterial;
class SteppingHelixPropagator;


class RoadSearchTrackCandidateMakerAlgorithm 
{
 public:
  
  RoadSearchTrackCandidateMakerAlgorithm(const edm::ParameterSet& conf);
  ~RoadSearchTrackCandidateMakerAlgorithm();

  /// Runs the algorithm
  void run(const RoadSearchCloudCollection* input,
	   const edm::Event& e,
	   const edm::EventSetup& es,
	   TrackCandidateCollection &output);

  //  edm::OwnVector<TrackingRecHit> 
  std::vector<TrajectoryMeasurement>  FindBestHitsByDet(const TrajectoryStateOnSurface& tsosBefore,
		      const std::set<const GeomDet*>& theDets,
		      edm::OwnVector<TrackingRecHit>& theHits);


  std::vector<TrajectoryMeasurement>  FindBestHit(const TrajectoryStateOnSurface& tsosBefore,
				     const std::set<const GeomDet*>& theDets,
				     edm::OwnVector<TrackingRecHit>& theHits);


  std::vector<TrajectoryMeasurement>  FindBestHits(const TrajectoryStateOnSurface& tsosBefore,
                                     const std::set<const GeomDet*>& theDets,
                                     edm::OwnVector<TrackingRecHit>& theHits);



 private:
  edm::ParameterSet conf_;

  unsigned int theNumHitCut;
  double theChi2Cut;
  bool NoFieldCosmic_;
  int MinChunkLength_;
  int nFoundMin_;

  std::string measurementTrackerName_;
  
  bool debug_;
  
  const MeasurementTracker*  theMeasurementTracker;
  const TrackerGeometry* geom;
  const TransientTrackingRecHitBuilder* ttrhBuilder;
  
  PropagatorWithMaterial* thePropagator;
  PropagatorWithMaterial* theRevPropagator;
  TrajectoryStateUpdator* theUpdator;
  MeasurementEstimator* theEstimator;
};

class CosmicCompareY {
 public:
  CosmicCompareY(const TrackerGeometry& tracker):_tracker(tracker){}
  bool operator()( const TrackingRecHit& rh1,
		   const TrackingRecHit& rh2) const
  {
    GlobalPoint gp1=_tracker.idToDet(rh1.geographicalId())->surface().toGlobal(rh1.localPosition());
    GlobalPoint gp2=_tracker.idToDet(rh2.geographicalId())->surface().toGlobal(rh2.localPosition());
    return gp1.y()<gp2.y();};
 private:
  const TrackerGeometry& _tracker;
};


#endif
