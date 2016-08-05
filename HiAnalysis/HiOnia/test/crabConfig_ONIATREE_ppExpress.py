from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'Run2015E-v1_Run_262163_262328_OniaTree_20151223'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'hioniaanalyzer_ppExpress_cfg.py'
config.JobType.outputFiles = ['OniaTree.root']

config.section_('Data')
config.Data.inputDataset = '/ExpressPhysics/anstahll-Run2015E-v1_Run_262163_262328_OniaSKIM_20151124_try3-14c622af79ae536f38372c873c91a5f4/USER'
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 100
config.Data.splitting = 'FileBased'
config.Data.outLFNDirBase = '/store/user/%s/ExpressStream/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = False

config.section_('Site')
config.Site.whitelist = ["T2_FR_GRIF_LLR"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

# If your site is blacklisted by crab, use:
# config.Data.ignoreLocality = True
# config.Site.whitelist = ["T2_FR*"]
