from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'HIRun2015-Express-v1_Run_263337_263729_ONIATREE_20151223'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'hioniaanalyzer_PbPbExpress_cfg.py'
config.JobType.outputFiles = ['OniaTree.root']

config.section_('Data')
config.Data.inputDataset = '/HIExpressPhysics/anstahll-HIRun2015-Express-v1_Run_263337_263729_OniaSKIM_20151212-678a6e571bb3afabeb026cc21bebb2ce/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 100
config.Data.splitting = 'FileBased'
config.Data.outLFNDirBase = '/store/user/%s/HIExpressStream/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = False

config.section_('Site')
config.Site.whitelist = ["T2_FR_GRIF_LLR"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

# If your site is blacklisted by crab, use:
# config.Data.ignoreLocality = True
# config.Site.whitelist = ["T2_FR*"]
