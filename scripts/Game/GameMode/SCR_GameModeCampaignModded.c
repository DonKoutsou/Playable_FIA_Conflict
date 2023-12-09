//------------------------------------------------------------------//
modded class SCR_GameModeCampaign
{
	[Attribute()]
	ref array <ref SP_CampaignBaseOverrideInfo> a_BaseOverrideInfo;
	
	[Attribute(defvalue : "150" ,desc : "patrols wich are closer than this value to a base will inherit that bases faction, if no bases withing this distance default back to INDfor")]
	int m_iBasePatrolInfluance
	
	int GetBasePatrolInfluance()
	{
		return m_iBasePatrolInfluance;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		// Sync respawn radios & control points amount
		int activeBasesTotal;

		reader.ReadInt(activeBasesTotal);

		m_BaseManager.SetTargetActiveBasesCount(activeBasesTotal);

		if (m_BaseManager.GetActiveBasesCount() == activeBasesTotal)
			m_BaseManager.OnAllBasesInitialized();

		int respawnRadiosBLUFOR, respawnRadiosOPFOR, respawnRadiosINFOR, controlPointsHeldBLUFOR, controlPointsHeldOPFOR, controlPointsHeldINFOR, primaryTargetBLUFOR, primaryTargetOPFOR, primaryTargetINFOR;

		reader.ReadInt(respawnRadiosBLUFOR);
		reader.ReadInt(respawnRadiosOPFOR);
		reader.ReadInt(respawnRadiosINFOR);

		reader.ReadInt(controlPointsHeldBLUFOR);
		reader.ReadInt(controlPointsHeldOPFOR);
		reader.ReadInt(controlPointsHeldINFOR);

		reader.ReadInt(primaryTargetBLUFOR);
		reader.ReadInt(primaryTargetOPFOR);
		reader.ReadInt(primaryTargetINFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetActiveRespawnRadios(respawnRadiosBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetActiveRespawnRadios(respawnRadiosOPFOR);
		GetFactionByEnum(SCR_ECampaignFaction.INDFOR).SetActiveRespawnRadios(respawnRadiosINFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetControlPointsHeld(controlPointsHeldBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetControlPointsHeld(controlPointsHeldOPFOR);
		GetFactionByEnum(SCR_ECampaignFaction.INDFOR).SetControlPointsHeld(controlPointsHeldINFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetBLUFOR)));
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetOPFOR)));
		GetFactionByEnum(SCR_ECampaignFaction.INDFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetINFOR)));

		return true;
	}
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		// Sync respawn radios & control points amount
		writer.WriteInt(m_BaseManager.GetTargetActiveBasesCount());

		int respawnRadiosBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetActiveRespawnRadios();
		int respawnRadiosOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetActiveRespawnRadios();
		int respawnRadiosINFOR = GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetActiveRespawnRadios();

		int controlPointsHeldBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetControlPointsHeld();
		int controlPointsHeldOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetControlPointsHeld();
		int controlPointsHeldINFOR = GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetControlPointsHeld();

		RplId primaryTargetBLUFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetPrimaryTarget());
		RplId primaryTargetOPFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetPrimaryTarget());
		RplId primaryTargetINFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetPrimaryTarget());

		writer.WriteInt(respawnRadiosBLUFOR);
		writer.WriteInt(respawnRadiosOPFOR);
		writer.WriteInt(respawnRadiosINFOR);

		writer.WriteInt(controlPointsHeldBLUFOR);
		writer.WriteInt(controlPointsHeldOPFOR);
		writer.WriteInt(controlPointsHeldINFOR);

		writer.WriteInt(primaryTargetBLUFOR);
		writer.WriteInt(primaryTargetOPFOR);
		writer.WriteInt(primaryTargetINFOR);

		return true;
	}
	//------------------------------------------------------------------------------------------------
	override protected void ApplyLoadedData()
	{
		m_BaseManager.GetOnAllBasesInitialized().Remove(ApplyLoadedData);

		if (!m_LoadedData)
			return;

		// Game was saved after match was over, don't load
		if (m_LoadedData.IsMatchOver())
			return;

		array<ref SCR_CampaignBaseStruct>basesStructs = m_LoadedData.GetBasesStructs();

		// No bases data available for load, something is wrong - terminate
		if (basesStructs.IsEmpty())
			return;

		m_bIsSessionLoadInProgress = true;
		m_BaseManager.LoadBasesStates(basesStructs);

		// We need to wait for all services to spawn before switching the progress bool to false so supplies are not deducted from bases
		GetGame().GetCallqueue().CallLater(EndSessionLoadProgress, DEFAULT_DELAY * 2);

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			m_BaseManager.InitializeSupplyDepotIcons();
			m_BaseManager.HideUnusedBaseIcons();
		}

		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.INDFOR));

		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();

		// Weather has to be changed after init
		if (timeHandler)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, DEFAULT_DELAY, false, m_LoadedData.GetHours(), m_LoadedData.GetMinutes(), m_LoadedData.GetSeconds(), m_LoadedData.GetWeatherState(), true);
		}

		SCR_CampaignTutorialArlandComponent tutorial = SCR_CampaignTutorialArlandComponent.Cast(FindComponent(SCR_CampaignTutorialArlandComponent));

		if (tutorial)
		{
			tutorial.SetResumeStage(m_LoadedData.GetTutorialStage());
			return;
		}

		m_iCallsignOffset = m_LoadedData.GetCallsignOffset();
		Replication.BumpMe();

		LoadRemnantsStates(m_LoadedData.GetRemnantsStructs());
		LoadClientData(m_LoadedData.GetPlayersStructs());

		SCR_CampaignFaction factionBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignFaction factionINDFOR = GetFactionByEnum(SCR_ECampaignFaction.INDFOR);

		// Delayed spawns to avoid calling them during init
		if (factionBLUFOR && m_LoadedData.GetMHQLocationBLUFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionBLUFOR, m_LoadedData.GetMHQLocationBLUFOR(), m_LoadedData.GetMHQRotationBLUFOR());

		if (factionOPFOR && m_LoadedData.GetMHQLocationOPFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionOPFOR, m_LoadedData.GetMHQLocationOPFOR(), m_LoadedData.GetMHQRotationOPFOR());
		
		if (factionINDFOR && m_LoadedData.GetMHQLocationINDFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionINDFOR, m_LoadedData.GetMHQLocationINDFOR(), m_LoadedData.GetMHQRotationINDFOR());

		m_LoadedData = null;
	}
	//------------------------------------------------------------------------------------------------
	//! Find out if any faction has won and it's time to end the match
	override protected void CheckForWinner()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		#ifndef AR_CAMPAIGN_TIMESTAMP
		float lowestVictoryTimestamp = float.MAX;
		float blockPauseTimestamp;
		float actualVictoryTimestamp;
		#else
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		WorldTimestamp lowestVictoryTimestamp;
		WorldTimestamp blockPauseTimestamp;
		WorldTimestamp actualVictoryTimestamp;
		#endif
		SCR_CampaignFaction winner;

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable() || fCast.GetFactionKey() == m_sINDFORFactionKey)
				continue;

			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();

			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = Replication.Time() + fCast.GetVictoryTimestamp() - fCast.GetPauseByBlockTimestamp();

			if (actualVictoryTimestamp != 0 && actualVictoryTimestamp < lowestVictoryTimestamp)
			{
				lowestVictoryTimestamp = actualVictoryTimestamp;
				winner = fCast;
			}
			#else
			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = curTime.PlusMilliseconds(
					fCast.GetVictoryTimestamp().DiffMilliseconds(fCast.GetPauseByBlockTimestamp())
				);

			if (actualVictoryTimestamp != 0)
			{
				if (!winner || actualVictoryTimestamp.Less(lowestVictoryTimestamp))
				{
					lowestVictoryTimestamp = actualVictoryTimestamp;
					winner = fCast;
				}
			}
			#endif
		}

		if (winner)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			if (lowestVictoryTimestamp <= Replication.Time())
			#else
			if (lowestVictoryTimestamp.LessEqual(curTime))
			#endif
			{
				GetGame().GetCallqueue().Remove(CheckForWinner);
				int winnerId = factionManager.GetFactionIndex(winner);
				RPC_DoEndMatch(winnerId);
				Rpc(RPC_DoEndMatch, winnerId);
				OnMatchSituationChanged();
			}
			else if (factionManager.GetFactionIndex(winner) != m_iWinningFactionId || winner.GetVictoryTimestamp() != m_fVictoryTimestamp || winner.GetPauseByBlockTimestamp() != m_fVictoryPauseTimestamp)
			{
				m_iWinningFactionId = factionManager.GetFactionIndex(winner);
				m_fVictoryTimestamp = winner.GetVictoryTimestamp();
				m_fVictoryPauseTimestamp = winner.GetPauseByBlockTimestamp();
				OnMatchSituationChanged();
				Replication.BumpMe();
			}
		}
		else if (m_iWinningFactionId != -1 || m_fVictoryTimestamp != 0)
		{
			m_iWinningFactionId = -1;
			#ifndef AR_CAMPAIGN_TIMESTAMP
			m_fVictoryTimestamp = 0;
			m_fVictoryPauseTimestamp = 0;
			#else
			m_fVictoryTimestamp = null;
			m_fVictoryPauseTimestamp = null;
			#endif
			OnMatchSituationChanged();
			Replication.BumpMe();
		}
	}
	//------------------------------------------------------------------------------------------------
	override protected void Start()
	{
		foreach (SP_CampaignBaseOverrideInfo overrideinfo :a_BaseOverrideInfo )
		{
			IEntity Base = GetGame().FindEntity(overrideinfo.m_sBaseToOverride);
			if (!Base)
				continue;
			SCR_CampaignMilitaryBaseComponent basecomp = SCR_CampaignMilitaryBaseComponent.Cast(Base.FindComponent(SCR_CampaignMilitaryBaseComponent));
			if (!basecomp)
				continue;
			if (overrideinfo.m_sFaction)
				basecomp.SetFaction(GetFactionByEnum(overrideinfo.m_sFaction));
			if (overrideinfo.m_iRadius)
				basecomp.SetRadius(overrideinfo.m_iRadius);
			if (overrideinfo.m_sName)
				basecomp.SetBaseName(overrideinfo.m_sName);
			if (overrideinfo.m_sAreaDesc)
				basecomp.SetAreaDesc(overrideinfo.m_sAreaDesc);
			//basecomp.SetCanBeHQ(overrideinfo.m_bCanBeHQ);
			//basecomp.SetIsControlPoint(overrideinfo.m_bIsControlPoint);
		}
		super.Start();
		
	}
	
};