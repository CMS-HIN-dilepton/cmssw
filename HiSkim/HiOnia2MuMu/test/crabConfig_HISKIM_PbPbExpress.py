from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'HIRun2015-Express-v1_Run_263337_263757_ONIASKIM_20151223'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'onia2MuMuPATHI_7xy_PbPbExpress_cfg.py'

config.section_('Data')
config.Data.inputDataset ='/HIExpressPhysics/HIRun2015-Express-v1/FEVT'  
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 1
config.Data.splitting = 'LumiBased'
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/HI/DCSOnly/json_DCSONLY.txt'
config.Data.runRange = '263337-263757'
config.Data.outLFNDirBase = '/store/user/%s/HIExpressStream/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = True
config.Data.outputDatasetTag = config.General.requestName

config.section_('Site')
config.Site.storageSite = 'T2_FR_GRIF_LLR'

