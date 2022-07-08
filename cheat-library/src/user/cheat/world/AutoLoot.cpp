#include "pch-il2cpp.h"
#include "AutoLoot.h"

#include <helpers.h>
#include <cheat/events.h>
#include <cheat/game/EntityManager.h>
#include <cheat/game/filters.h>
#include <cheat/game/Chest.h>

namespace cheat::feature 
{
	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);
	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method);

    AutoLoot::AutoLoot() : Feature(),
        NF(f_AutoPickup,     "Auto-pickup drops",               "AutoLoot", true),
		NF(f_AutoTreasure,   "Auto-open treasures",             "AutoLoot", true),
		NF(f_UseCustomRange, "Use custom pickup range",         "AutoLoot", true),
		NF(f_PickupFilter,	 "Pickup filter",					"AutoLoot", false),
		NF(f_PickupFilter_Animals,	 "Animals filter",			"AutoLoot", true),
		NF(f_PickupFilter_DropItems, "Drop items filter",		"AutoLoot", true),
		NF(f_PickupFilter_Resources, "Resources filter",		"AutoLoot", true),
		NF(f_Chest,			 "Chests",							"AutoLoot", false),
		NF(f_Leyline,		 "Leylines",						"AutoLoot", false),
		NF(f_Investigate,	 "Search points",					"AutoLoot", false),
		NF(f_QuestInteract,  "Quest interacts",					"AutoLoot", false),
        NF(f_Others,		 "Other treasures",					"AutoLoot", false),
		NF(f_DelayTime,		 "Delay time (in ms)",				"AutoLoot", 280),
        NF(f_CustomRange,    "Pickup Range",                    "AutoLoot", 40.0f),
		toBeLootedItems(), nextLootTime(0)
    {
		// Auto loot
		HookManager::install(app::MoleMole_LCSelectPickup_AddInteeBtnByID, LCSelectPickup_AddInteeBtnByID_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsInPosition, LCSelectPickup_IsInPosition_Hook);
		HookManager::install(app::MoleMole_LCSelectPickup_IsOutPosition, LCSelectPickup_IsOutPosition_Hook);

		events::GameUpdateEvent += MY_METHOD_HANDLER(AutoLoot::OnGameUpdate);
	}

    const FeatureGUIInfo& AutoLoot::GetGUIInfo() const
    {
        static const FeatureGUIInfo info{ "Auto Loot", "World", true };
        return info;
    }

    void AutoLoot::DrawMain()
    {
		if (ImGui::BeginTable("AutoLootDrawTable", 2, ImGuiTableFlags_NoBordersInBody))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::BeginGroupPanel("Pickup");
			{
				ConfigWidget("Auto-Pickup", f_AutoPickup, "Automatically picks up dropped items.\n" \
					"Note: Using this with custom range and low delay times is extremely risky.\n" \
					"Abuse will definitely merit a ban.\n\n" \
					"If using with custom range, make sure this is turned on FIRST.");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(254, 89, 0, 255), "Read the note!");
				ImGui::Spacing();
				ImGui::Spacing();

			}
			
			{
				ConfigWidget("Custom Pickup Range", f_UseCustomRange, "Enable custom pickup range.\n" \
					"High values are not recommended, as it is easily detected by the server.\n\n" \
					"If using with auto-pickup/auto-treasure, turn this on LAST.");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(254, 89, 0, 255), "Read the note!");
				ImGui::SetNextItemWidth(185.0f);
				ConfigWidget("Range (m)", f_CustomRange, 0.1f, 0.5f, 40.0f, "Modifies pickup/open range to this value (in meters).");
			}

			ImGui::EndGroupPanel();
			{
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ConfigWidget("Auto-Treasure", f_AutoTreasure, "Automatically opens chests and other treasures.\n" \
					"Note: Using this with custom range and low delay times is extremely risky.\n" \
					"Abuse will definitely merit a ban.\n\n" \
					"If using with custom range, make sure this is turned on FIRST.");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(254, 89, 0, 255), "Read the note!");
				ImGui::Indent();
				ConfigWidget("Chests", f_Chest, "Common, precious, luxurious, etc.");
				ConfigWidget("Search Points", f_Investigate, "Marked as Investigate/Search, etc.");
				ImGui::SameLine();
				ConfigWidget("Leyline", f_Leyline, "Mora/XP, overworld/Trounce bosses, etc.");
			    ConfigWidget("Quest Interacts", f_QuestInteract, "Valid quest interact points.");
				ImGui::SameLine();
				ConfigWidget("Others", f_Others, "Book Pages, Spincrystals, etc.");
				ImGui::Unindent();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::BeginGroupPanel("Looting Speed");
			{
				ImGui::SetNextItemWidth(100.0f);
				ConfigWidget("Delay Time (ms)", f_DelayTime, 1, 0, 1000, "Delay (in ms) between loot/open actions.\n" \
					"Values under 200ms are unsafe.\nNot used if no auto-functions are on.");
				ImGui::Spacing();
				ImGui::Spacing();
				ConfigWidget("Pickup Filter", f_PickupFilter, "Enable pickup filter.\n");
				ConfigWidget("Animals", f_PickupFilter_Animals, "Fish, Lizard, Frog, Flying animals."); ImGui::SameLine();
				ConfigWidget("Drop Items", f_PickupFilter_DropItems, "Material, Mineral, Artifact."); ImGui::SameLine();
				ConfigWidget("Resources", f_PickupFilter_Resources, "Everything beside Animals and Drop Items (Plants, Books, etc).");
			}
			ImGui::EndGroupPanel();
			
			
			
			
			ImGui::EndTable();
		}
			
    	
    }

    bool AutoLoot::NeedStatusDraw() const
	{
        return f_AutoPickup || f_AutoTreasure || f_UseCustomRange || f_PickupFilter;
    }

    void AutoLoot::DrawStatus() 
    {
		ImGui::Text("Auto Loot\n[%s%s%s%s%s]",
			f_AutoPickup ? "AP" : "",
			f_AutoTreasure ? fmt::format("{}AT", f_AutoPickup ? "|" : "").c_str() : "",
			f_UseCustomRange ? fmt::format("{}CR{:.1f}m", f_AutoPickup || f_AutoTreasure ? "|" : "", f_CustomRange.value()).c_str() : "",
			f_PickupFilter ? fmt::format("{}PF", f_AutoPickup || f_AutoTreasure || f_UseCustomRange ? "|" : "").c_str() : "",
			f_AutoPickup || f_AutoTreasure ? fmt::format("|{}ms", f_DelayTime.value()).c_str() : ""
		);
    }

    AutoLoot& AutoLoot::GetInstance()
    {
        static AutoLoot instance;
        return instance;
    }

	bool AutoLoot::OnCreateButton(app::BaseEntity* entity)
	{
		if (!f_AutoPickup)
			return false;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return false;
    	
		auto entityId = entity->fields._runtimeID_k__BackingField;
		if (f_DelayTime == 0)
		{
			app::MoleMole_ItemModule_PickItem(itemModule, entityId, nullptr);
			return true;
		}

		toBeLootedItems.push(entityId);
		return false;
	}

	void AutoLoot::OnGameUpdate()
	{
		auto currentTime = util::GetCurrentTimeMillisec();
		if (currentTime < nextLootTime)
			return;

		auto entityManager = GET_SINGLETON(MoleMole_EntityManager);
		if (entityManager == nullptr)
			return;

		// RyujinZX#6666
		if (f_AutoTreasure) 
		{
			auto& manager = game::EntityManager::instance();
			for (auto& entity : manager.entities(game::filters::combined::Chests)) 
			{
				float range = f_UseCustomRange ? f_CustomRange : 3.5f;
				if (manager.avatar()->distance(entity) >= range)
					continue;

				auto chest = reinterpret_cast<game::Chest*>(entity);
				auto chestType = chest->itemType();

				if (!f_Investigate && chestType == game::Chest::ItemType::Investigate)
					continue;

				if (!f_QuestInteract && chestType == game::Chest::ItemType::QuestInteract)
					continue;

				if (!f_Others && (
					chestType == game::Chest::ItemType::BGM ||
					chestType == game::Chest::ItemType::BookPage
					))
					continue;

				if (!f_Leyline && chestType == game::Chest::ItemType::Flora)
					continue;

				if (chestType == game::Chest::ItemType::Chest)
				{
					if (!f_Chest)
						continue;
					auto ChestState = chest->chestState();
					if (ChestState != game::Chest::ChestState::None)
						continue;
				}

				uint32_t entityId = entity->runtimeID();
				toBeLootedItems.push(entityId);
			}
		}

		auto entityId = toBeLootedItems.pop();
		if (!entityId)
			return;

		auto itemModule = GET_SINGLETON(MoleMole_ItemModule);
		if (itemModule == nullptr)
			return;

		auto entity = app::MoleMole_EntityManager_GetValidEntity(entityManager, *entityId, nullptr);
		if (entity == nullptr)
			return;

		app::MoleMole_ItemModule_PickItem(itemModule, *entityId, nullptr);
		nextLootTime = currentTime + (int)f_DelayTime;
	}

	void AutoLoot::OnCheckIsInPosition(bool& result, app::BaseEntity* entity)
	{
		if (f_AutoPickup || f_UseCustomRange) {
			float pickupRange = f_UseCustomRange ? f_CustomRange : 3.5f;
			if (f_PickupFilter)
			{
				if (!f_PickupFilter_Animals && entity->fields.entityType == app::EntityType__Enum_1::EnvAnimal ||
					!f_PickupFilter_DropItems && entity->fields.entityType == app::EntityType__Enum_1::DropItem ||
					!f_PickupFilter_Resources && entity->fields.entityType == app::EntityType__Enum_1::GatherObject)
				{
					result = false;
					return;
				}
			}
			
			auto& manager = game::EntityManager::instance();
			result = manager.avatar()->distance(entity) < pickupRange;
		}
	}

	static void LCSelectPickup_AddInteeBtnByID_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		AutoLoot& autoLoot = AutoLoot::GetInstance();
		bool canceled = autoLoot.OnCreateButton(entity);
		if (!canceled)
			CALL_ORIGIN(LCSelectPickup_AddInteeBtnByID_Hook, __this, entity, method);
	}

	static bool LCSelectPickup_IsInPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsInPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}

	static bool LCSelectPickup_IsOutPosition_Hook(void* __this, app::BaseEntity* entity, MethodInfo* method)
	{
		bool result = CALL_ORIGIN(LCSelectPickup_IsOutPosition_Hook, __this, entity, method);

		AutoLoot& autoLoot = AutoLoot::GetInstance();
		autoLoot.OnCheckIsInPosition(result, entity);

		return result;
	}
}

