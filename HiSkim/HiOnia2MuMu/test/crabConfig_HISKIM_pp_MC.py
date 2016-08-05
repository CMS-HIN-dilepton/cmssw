from CRABClient.UserUtilities import config, getUsernameFromSiteDB
config = config()

config.section_('General')
config.General.requestName = 'JpsiMM_5p02TeV_TuneCUETP8M1_ONIASKIM_20151222'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'onia2MuMuPATHI_7xy_pp_MC_cfg.py'

config.section_('Data')
config.Data.inputDataset ='/JpsiMM_5p02TeV_TuneCUETP8M1/miheejo-JpsiMM_5p02TeV_TuneCUETP8M1_RAW2DIGI_L1Reco_RECO-eca985b12211699dc9125db438586ff6/USER'  
config.Data.inputDBS = 'phys03'
config.Data.unitsPerJob = 1
config.Data.splitting = 'FileBased'
config.Data.outLFNDirBase = '/store/user/%s/ppMC2015/%s' % (getUsernameFromSiteDB(), config.General.requestName)
config.Data.publication = True
config.Data.outputDatasetTag = config.General.requestName

config.section_('Site')
config.Site.whitelist = ['T2_FR_GRIF_LLR']
config.Site.storageSite = 'T2_FR_GRIF_LLR'

