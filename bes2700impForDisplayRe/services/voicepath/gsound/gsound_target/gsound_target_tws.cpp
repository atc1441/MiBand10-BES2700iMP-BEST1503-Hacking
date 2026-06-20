#include "gsound_dbg.h"
#include "gsound_target.h"
#include "gsound_custom_tws.h"

void GSoundTargetTwsRoleChangeResponse(bool accepted)
{
    gsound_tws_on_roleswitch_accepted_by_lib(accepted);
}

void GSoundTargetTwsRoleChangeGSoundDone(void)
{
    GLOG_D("[%s]+++", __func__);

    gsound_tws_roleswitch_lib_complete();
}

void GSoundTargetTwsInit(const GSoundTwsInterface *handler)
{
    gsound_tws_register_target_handle(handler);

    gsound_tws_init();
}
