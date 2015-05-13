#!/bin/bash -

source /afs/cern.ch/cms/LCG/LCG-2/UI/cms_ui_env.sh
voms-proxy-init --valid 168:00 -voms cms 
#-pwstdin < $HOME/.grid-cert-passphrase
voms-proxy-info --all
source /afs/cern.ch/cms/sw/cmsset_default.sh
#todo to put on afs
cd /afs/cern.ch/cms/CAF/CMSPHYS/PHYS_HEAVYIONS/cms/CMSSW_3_9_2_patch3/src
eval `scram runtime -sh`
source /afs/cern.ch/cms/ccs/wm/scripts/Crab/crab.sh
cd -
export STAGE_SVCCLASS=cmscaf 
