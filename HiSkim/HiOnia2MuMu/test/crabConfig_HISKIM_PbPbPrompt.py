from CRABClient.UserUtilities import config, getUsernameFromSiteDB
from WMCore.DataStructs.LumiList import LumiList

config = config()

config.section_('General')
config.General.requestName = 'HIOniaL1DoubleMu0_HIRun2015-PromptReco-v1_Run_262548_263757_ONIASKIM_20151222'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = True

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'onia2MuMuPATHI_7xy_PbPbPrompt_cfg.py'

config.section_('Data')
config.Data.inputDataset ='/HIOniaL1DoubleMu0/HIRun2015-PromptReco-v1/AOD'
config.Data.inputDBS = 'global'
config.Data.unitsPerJob = 10
config.Data.splitting = 'LumiBased'
config.Data.runRange = '262548-263757'


### Use when running firts time
config.Data.lumiMask = '/afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions15/HI/DCSOnly/json_DCSONLY.txt'
### When submiting the jobs again, please use: 
#config.Data.lumiMask = '<NAME OF MISSING LUMI MASK FROM PREVIOUS CRAB JOB>'
# The missing lumimask can be obtain after using crab report -d <path to crab job dir> 


config.Data.publication = True
config.Data.outputDatasetTag = 'HIOniaL1DoubleMu0_HIRun2015-PromptReco-v1_Run_262548_263757_ONIASKIM'
config.Data.outLFNDirBase = '/store/user/%s/HIPromptReco/%s' % (getUsernameFromSiteDB(), config.Data.outputDatasetTag)

config.section_('Site')
config.Site.whitelist = ["T1_FR*"]
config.Site.storageSite = 'T2_FR_GRIF_LLR'

