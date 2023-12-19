modded class SCR_CampaignMilitaryBaseComponent : SCR_MilitaryBaseComponent
{
	[Attribute("200")]
	protected int m_iStartingSupplies;
	
	[Attribute("38", UIWidgets.Slider, "Radio frequency (MHz) for players operating at this base (FIA)", "38 54 2", category: "Campaign")]
	protected int m_iFreqIND;
	
	[RplProp(onRplName: "OnHasSignalChanged")]
	protected SCR_ECampaignHQRadioComms m_eRadioCoverageINFOR = SCR_ECampaignHQRadioComms.NONE;
	
	[Attribute("", UIWidgets.Coords, params: "inf inf inf purpose=coords space=world", desc: "")]
	vector m_BasePossitionOverride;	
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Override HQ composition in small bases", "et")]
	private ResourceName m_OverrideBaseBuildingHQ;
	
	[Attribute(desc: "Description of area for event keeping purposes")]
	string m_sAreaDesc;
	
	[Attribute(defvalue : "200", desc: "Description of area for event keeping purposes")]
	int m_iBasePatrolInfluance;

	SCR_CampaignFaction INDFOR;
	
	protected ref map <Faction, SCR_ECampaignHQRadioComms > m_eFactionRadioCoverage;
	
	
	
	
	string GetAreaDesc()
	{
		return m_sAreaDesc;
	}
	void SetAreaDesc(string text)
	{
		m_sAreaDesc = text;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStartingSupplies()
	{
		return m_iStartingSupplies;
	}
	void SetBaseName(string name)
	{
		m_sBaseName = name;
	}
	override void OnAllBasesInitialized()
	{
		if (IsProxy())
		{
			UpdateBasesInRadioRange();
			return;
		}

		// Spawn HQ composition
		if (GetType() == SCR_ECampaignBaseType.BASE)
		{
			ResourceName buildingPrefab; 
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			if ( m_OverrideBaseBuildingHQ )
				buildingPrefab = m_OverrideBaseBuildingHQ;
			else
			{
				buildingPrefab = GetCampaignFaction().GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
			}
			

			if (!buildingPrefab)
			{
				Math.Randomize(-1);

				if (Math.RandomFloat01() >= 0.5)
					buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
				else
					buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetBuildingPrefab(EEditableEntityLabel.SERVICE_HQ);
			}
			vector pos;
			if (m_BasePossitionOverride > vector.Zero)
				pos = m_BasePossitionOverride;
			else
				pos = GetOwner().GetOrigin();
			
			if (buildingPrefab)
				GetGame().GetCallqueue().CallLater(SpawnBuilding, 1000, false, buildingPrefab, pos, GetOwner().GetYawPitchRoll(), true);	// Delay so we don't spawn stuff during init
		}
	}
	override void SetHQRadioConverage(notnull SCR_Faction faction, SCR_ECampaignHQRadioComms coverage)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		m_eFactionRadioCoverage.Set(faction, coverage);
		
		/*if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
		{
			if (m_eRadioCoverageBLUFOR == coverage)
				return;

			m_eRadioCoverageBLUFOR = coverage;
		}
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
		{
			if (m_eRadioCoverageOPFOR == coverage)
				return;

			m_eRadioCoverageOPFOR = coverage;
		}
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR))
		{
			if (m_eRadioCoverageINFOR == coverage)
				return;

			m_eRadioCoverageINFOR = coverage;
		}*/
		

		Replication.BumpMe();
		OnHasSignalChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ECampaignHQRadioComms GetHQRadioCoverage(notnull Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return SCR_ECampaignHQRadioComms.NONE;

		/*if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			return m_eRadioCoverageBLUFOR;
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
			return m_eRadioCoverageOPFOR;
		else if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR))
			return m_eRadioCoverageINFOR;*/
		

		return m_eFactionRadioCoverage.Get(faction);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsHQRadioTrafficPossible(Faction faction, SCR_ECampaignHQRadioComms direction = SCR_ECampaignHQRadioComms.RECEIVE)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return false;

		//bool isBLUFOR = (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		//bool isINDFOR = (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
		//if (!isBLUFOR && faction != campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR))
			//return false;
		
		SCR_ECampaignHQRadioComms RadioCoverage = m_eFactionRadioCoverage.Get(faction);
		
		switch (direction)
		{
			case SCR_ECampaignHQRadioComms.RECEIVE:
			{
				return (RadioCoverage == SCR_ECampaignHQRadioComms.RECEIVE || RadioCoverage == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				/*if (isINDFOR)
					return (m_eRadioCoverageINFOR == SCR_ECampaignHQRadioComms.RECEIVE || m_eRadioCoverageINFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				if (isBLUFOR)
					return (m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.RECEIVE || m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				else
					return (m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.RECEIVE || m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);*/

				break;
			}

			case SCR_ECampaignHQRadioComms.SEND:
			{
				return (RadioCoverage == SCR_ECampaignHQRadioComms.SEND || RadioCoverage == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				
				/*if (isINDFOR)
					return (m_eRadioCoverageINFOR == SCR_ECampaignHQRadioComms.SEND || m_eRadioCoverageINFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				if (isBLUFOR)
					return (m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.SEND || m_eRadioCoverageBLUFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);
				else
					return (m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.SEND || m_eRadioCoverageOPFOR == SCR_ECampaignHQRadioComms.BOTH_WAYS);*/
				break;
			}
		}

		//if (isBLUFOR)
		//	return (direction == m_eRadioCoverageBLUFOR);
		//if (isINDFOR)
		//	return (direction == m_eRadioCoverageINFOR);
		//return (direction == m_eRadioCoverageOPFOR);
		
		return (direction == RadioCoverage);
	}

	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the owning faction changes
	override protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		if (!GetGame().InPlayMode())
			return;

		SCR_CampaignFaction newCampaignFaction = SCR_CampaignFaction.Cast(faction);

		if (!newCampaignFaction)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();

		if (!IsProxy())
		{
			// Update signal coverage only if the base was seized during normal play, not at the start
			if (curTime > 10000)
			{
				campaign.GetBaseManager().RecalculateRadioCoverage(campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
			}
		}
		super.OnFactionChanged(owner, previousFaction, faction);
	}
	//------------------------------------------------------------------------------------------------
	override int GetRadioFrequency(FactionKey faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return INVALID_FREQUENCY;

		switch (faction)
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR): {return m_iFreqWest; };
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR): {return m_iFreqEast; };
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR): {return m_iFreqIND; };
		}

		return INVALID_FREQUENCY;
	}
	//------------------------------------------------------------------------------------------------
	override void OnRadioRangeChanged()
	{
		UpdateBasesInRadioRange();

		if (m_MapDescriptor)
			m_MapDescriptor.HandleMapLinks();

		if (IsProxy())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
		SCR_CampaignMilitaryBaseManager bManager = campaign.GetBaseManager();

		if (GetGame().GetWorld().GetWorldTime() > campaign.BACKEND_DELAY)
		{
			// Process recalculation immediately unless we're still within save loading period
			bManager.RecalculateRadioCoverage(GetCampaignFaction());
		}
		else
		{
			GetGame().GetCallqueue().Remove(bManager.RecalculateRadioCoverage);
			SCR_CampaignFactionManager factman = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
			array < Faction > factions = {};
			factman.GetFactionsList(factions);
			foreach (Faction faction : factions)
			{
				SCR_CampaignFaction cfaction = SCR_CampaignFaction.Cast(faction);
				GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, cfaction);
			}
			// Otherwise process for both factions only once so we're not doing it for each antenna loaded
			
			//GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			//GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			//GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
		}																																						 

		RefreshTasks();
	};
	//------------------------------------------------------------------------------------------------
	override void SetFaction(SCR_CampaignFaction faction)
	{
		if (IsProxy())
			return;

		if (!m_FactionComponent)
			return;

		m_FactionComponent.SetAffiliatedFaction(faction);
		IEntity Child = GetOwner().GetChildren();
		if (Child && Child.FindComponent(SCR_AmbientPatrolSpawnPointComponent))
		{
			SCR_FactionAffiliationComponent affcomp = SCR_FactionAffiliationComponent.Cast(Child.FindComponent(SCR_FactionAffiliationComponent));
			affcomp.SetFaction(Child, faction);
		}
		else
		{
			Child = Child.GetSibling();
			if (Child && Child.FindComponent(SCR_AmbientPatrolSpawnPointComponent))
			{
				SCR_FactionAffiliationComponent affcomp = SCR_FactionAffiliationComponent.Cast(Child.FindComponent(SCR_FactionAffiliationComponent));
				affcomp.SetFaction(Child, faction);
			}
		}
	}
	void SetCanBeHQ(bool toset)
	{
		m_bCanBeHQ = toset;
	}
	void SetIsControlPoint(bool toset)
	{
		m_bIsControlPoint = toset;
	}
	override protected void OnLocalPlayerFactionAssigned(Faction assignedFaction)
	{
		UpdateBasesInRadioRange();
		//m_MapDescriptor.MapSetup(assignedFaction);
		//m_MapDescriptor.HandleMapInfo(SCR_CampaignFaction.Cast(assignedFaction));
		//HideMapLocationLabel();
	}
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_CampaignFactionManager factman = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		array < Faction > factions = {};
		factman.GetFactionsList(factions);
		m_eFactionRadioCoverage = new map <Faction, SCR_ECampaignHQRadioComms> ();
		foreach (Faction faction : factions)
		{
			m_eFactionRadioCoverage.Insert(faction, 0);
		}
		super.OnPostInit(owner);
	}
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}
};
modded class SCR_MapCampaignUI
{
	override protected void InitBases()
	{
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignMilitaryBaseManager CbaseManager = campaign.GetBaseManager();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base;
		
		for (int i = 0, count = bases.Count(); i < count; ++i)
		{
			base = SCR_CampaignMilitaryBaseComponent.Cast(bases[i]);
			
			if (!base || !base.IsInitialized())
				continue;
			
			if (base.IsHQ() && base.GetFaction().IsFactionEnemy(faction))
				continue;

			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sBaseElement, m_wIconsContainer);
			SCR_CampaignMapUIBase handler = SCR_CampaignMapUIBase.Cast(w.FindHandler(SCR_CampaignMapUIBase));
			
			if (!handler)
				return;

			handler.SetParent(this);
			handler.InitBase(base);
			m_mIcons.Set(w, handler);
			base.SetBaseUI(handler);

			FrameSlot.SetSizeToContent(w, true);
			FrameSlot.SetAlignment(w, 0.5, 0.5);
		}

		if (faction)
		{
			string factionKey = faction.GetFactionKey();
			InitMobileAssembly(factionKey, faction.GetMobileAssembly() != null);
		}
		
		UpdateIcons();
	}
	
	/*override void OnMapOpen(MapConfiguration config)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (m_PlyFactionAffilComp)
				m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);	

			m_PlyGroupComp = SCR_PlayerControllerGroupComponent.Cast(pc.FindComponent(SCR_PlayerControllerGroupComponent));
			if (m_PlyGroupComp)
				m_PlyGroupComp.GetOnGroupChanged().Insert(OnPlayerGroupChanged);		
		}
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(AddSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(RemoveSpawnPoint);

		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskAdded);

		m_bIsDeployMap = (config.MapEntityMode == EMapEntityMode.SPAWNSCREEN);

		m_wIconsContainer = m_RootWidget.FindAnyWidget(m_sIconsContainer);
		m_wIconsContainer.SetVisible(true);

		Widget child = m_wIconsContainer.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			delete child;
			child = sibling;
		}

		if (m_bShowSpawnPoints)
			InitSpawnPoints();

		if (m_bShowTasks)
			InitTaskMarkers();
		
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);
	}*/
}
