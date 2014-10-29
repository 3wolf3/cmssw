import FWCore.ParameterSet.Config as cms

from DQM.OuterTrackerMonitorCluster.OuterTrackerMonitorCluster_cfi import *
from DQM.OuterTrackerMonitorStub.OuterTrackerMonitorStub_cfi import *

OuterTrackerSource = cms.Sequence(OuterTrackerMonitorCluster * 
				  OuterTrackerMonitorStub)
