//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
modded class SCR_CampaignStruct
{

	protected vector m_vMHQLocationINDFOR;
	protected vector m_vMHQRotationINDFOR;

	//------------------------------------------------------------------------------------------------
	vector GetMHQLocationINDFOR()
	{
		return m_vMHQLocationINDFOR;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetMHQRotationINDFOR()
	{
		return m_vMHQRotationINDFOR;
	}
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return false;
		
		m_bMatchOver = campaign.GetIsMatchOver();
		
		Clear();
		
		ChimeraWorld world = campaign.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		
		if (timeManager)
		{
			timeManager.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
			WeatherStateTransitionManager transitionManager = timeManager.GetTransitionManager();
			
			if (transitionManager)
				m_sWeatherState = transitionManager.GetCurrentState().GetStateName();
		}
		
		campaign.GetBaseManager().StoreBasesStates(m_aBasesStructs);
		
		SCR_CampaignTutorialArlandComponent tutorial = SCR_CampaignTutorialArlandComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialArlandComponent));
		
		if (tutorial)
		{
			m_iTutorialStage = tutorial.GetStage();
			return true;
		}
		
		campaign.StoreRemnantsStates(m_aRemnantsStructs);
		
		SCR_CampaignFaction factionBLUFOR = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignFaction factionINDFOR = campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR);
		IEntity mobileHQ;
		
		if (factionBLUFOR && factionBLUFOR.GetMobileAssembly())
		{
			mobileHQ = factionBLUFOR.GetMobileAssembly().GetVehicle();
			
			if (mobileHQ)
			{
				m_vMHQLocationBLUFOR = mobileHQ.GetOrigin();
				m_vMHQRotationBLUFOR = mobileHQ.GetYawPitchRoll();
			}
		}
		
		if (factionOPFOR && factionOPFOR.GetMobileAssembly())
		{
			mobileHQ = factionOPFOR.GetMobileAssembly().GetVehicle();
			
			if (mobileHQ)
			{
				m_vMHQLocationOPFOR = mobileHQ.GetOrigin();
				m_vMHQRotationOPFOR = mobileHQ.GetYawPitchRoll();
			}
		}
		if (factionINDFOR && factionINDFOR.GetMobileAssembly())
		{
			mobileHQ = factionINDFOR.GetMobileAssembly().GetVehicle();
			
			if (mobileHQ)
			{
				m_vMHQLocationINDFOR = mobileHQ.GetOrigin();
				m_vMHQRotationINDFOR = mobileHQ.GetYawPitchRoll();
			}
		}
		campaign.WriteAllClientsData();
		array<ref SCR_CampaignClientData> clients = {};
		campaign.GetClientsData(clients);
		
		foreach (SCR_CampaignClientData data : clients)
		{
			SCR_CampaignPlayerStruct struct = new SCR_CampaignPlayerStruct();
			struct.SetID(data.GetID());
			struct.SetXP(data.GetXP());
			struct.SetFactionIndex(data.GetFactionIndex());
			
			m_aPlayerStructs.Insert(struct);
		}
		
		m_iCallsignOffset = campaign.GetCallsignOffset();
		
		return true;
	}
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		m_aBasesStructs.Clear();
		m_aRemnantsStructs.Clear();
		m_aPlayerStructs.Clear();
		m_vMHQLocationBLUFOR = vector.Zero;
		m_vMHQRotationBLUFOR = vector.Zero;
		m_vMHQLocationOPFOR = vector.Zero;
		m_vMHQRotationOPFOR = vector.Zero;
		m_vMHQLocationINDFOR = vector.Zero;
		m_vMHQRotationINDFOR = vector.Zero;
	}
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignStruct()
	{
		RegV("m_iHours");
		RegV("m_iMinutes");
		RegV("m_iSeconds");
		RegV("m_vMHQLocationBLUFOR");
		RegV("m_vMHQRotationBLUFOR");
		RegV("m_vMHQLocationOPFOR");
		RegV("m_vMHQRotationOPFOR");
		RegV("m_vMHQLocationINDFOR");
		RegV("m_vMHQRotationINDFOR");
		RegV("m_aBasesStructs");
		RegV("m_aRemnantsStructs");
		RegV("m_aPlayerStructs");
		RegV("m_iTutorialStage");
		RegV("m_bMatchOver");
		RegV("m_sWeatherState");
		RegV("m_iCallsignOffset");
	}
};
