from CRABClient.UserUtilities import config, getUsernameFromSiteDB
from WMCore.DataStructs.LumiList import LumiList

config = config()

config.section_('General')
config.General.requestName = 'DoubleMu_Run2015E-PromptReco-v1_Run_262081_262328_ONIASKIM_20151222'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'onia2MuMuPATHI_7xy_ppPrompt_cfg.py'

config.section_('Data')
config.Data.inputDataset ='/DoubleMu/Run2015E-PromptReco-v1/AOD'
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 10
config.Data.splitting = 'LumiBased'
config.Data.runRange = '262081-262328'


### Use when running firts time
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/5TeV/DCSONLY/json_DCSONLY.txt'
### When submiting the jobs again, please use: 
#config.Data.lumiMask = '<NAME OF MISSING LUMI MASK FROM PREVIOUS CRAB JOB>'
# The missing lumimask can be obtain after using crab report -d <path to crab job dir> 


config.Data.publication = True
config.Data.outputDatasetTag = 'DoubleMu_Run2015E-PromptReco-v1_Run_262081_262328_ONIASKIM'
config.Data.outLFNDirBase = '/store/user/%s/PromptReco/%s' % (getUsernameFromSiteDB(), config.Data.outputDatasetTag)

config.section_('Site')
config.Site.whitelist = ["T1_FR*"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

