import FWCore.ParameterSet.Config as cms

goodZToMuMuOneStandAloneMuonTrackMCMatch = cms.EDFilter("MCTruthCompositeMatcherNew",
    src = cms.InputTag("goodZToMuMuOneStandAloneMuonTrack"),
    matchPDGId = cms.vint32(),
    matchMaps = cms.VInputTag(cms.InputTag("goodMuonMCMatch"), cms.InputTag("goodStandAloneMuonTrackMCMatch"))
)


