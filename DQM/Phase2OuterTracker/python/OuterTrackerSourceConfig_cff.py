import FWCore.ParameterSet.Config as cms

from Geometry.TrackerGeometryBuilder.StackedTrackerGeometry_cfi import *
from DQM.Phase2OuterTracker.OuterTrackerMonitorCluster_cfi import *
from DQM.Phase2OuterTracker.OuterTrackerMonitorStub_cfi import *
from DQM.Phase2OuterTracker.OuterTrackerMonitorL1Track_cfi import *
from DQM.Phase2OuterTracker.OuterTrackerMonitorPixelDigiMaps_cfi import *

OuterTrackerSource = cms.Sequence(OuterTrackerMonitorCluster * 
				  OuterTrackerMonitorStub *
				  OuterTrackerMonitorL1Track *
				  OuterTrackerMonitorPixelDigiMaps )
