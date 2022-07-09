#include "pch-il2cpp.h"
#include "EnablePeaking.h"

#include <helpers.h>

namespace cheat::feature
{
    static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method);

    EnablePeaking::EnablePeaking() : Feature(),
        NF(f_Enabled, "Enable Peaking", "Visuals::EnablePeaking", false)
    {
        HookManager::install(app::MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue, MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook);
    }

    const FeatureGUIInfo& EnablePeaking::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ "EnablePeaking", "Visuals", false };
        return info;
    }

    void EnablePeaking::DrawMain()
    {
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ConfigWidget(f_Enabled, "look under skirt ;)");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
    }

    bool EnablePeaking::NeedStatusDraw() const
    {
        return f_Enabled;
    }

    void EnablePeaking::DrawStatus()
    {
        ImGui::Text("Enable Peaking");
    }

    EnablePeaking& EnablePeaking::GetInstance()
    {
        static EnablePeaking instance;
        return instance;
    } 

    static void MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook(app::MoleMole_VCBaseSetDitherValue* __this, float value, MethodInfo* method)
    {
        EnablePeaking& EnablePeaking = EnablePeaking::GetInstance();
        if (EnablePeaking.f_Enabled)
            value = 1;

        CALL_ORIGIN(MoleMole_VCBaseSetDitherValue_set_ManagerDitherAlphaValue_Hook, __this, value, method);
    }
}

