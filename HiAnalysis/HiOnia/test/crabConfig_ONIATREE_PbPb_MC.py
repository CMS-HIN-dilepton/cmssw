from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'JpsiMM_5p02TeV_TuneCUETP8M1_ptJpsi36_ONIATREE_20151216'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'hioniaanalyzer_PbPb_MC_cfg.py'
config.JobType.outputFiles = ['OniaTree.root']

config.section_('Data')
config.Data.inputDataset = '/JpsiMM_5p02TeV_TuneCUETP8M1_ptJpsi36/anstahll-JpsiMM_5p02TeV_TuneCUETP8M1_ptJpsi36_ONIASKIM_20151208-bf8f659f39d8e2edd88bff0358389916/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 100
config.Data.splitting = 'FileBased'
config.Data.outLFNDirBase = '/store/user/%s/PbPbMC2015/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = False

config.section_('Site')
config.Site.whitelist = ["T2_FR_GRIF_LLR"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

# If your site is blacklisted by crab, use:
# config.Data.ignoreLocality = True
# config.Site.whitelist = ["T2_FR*"]

