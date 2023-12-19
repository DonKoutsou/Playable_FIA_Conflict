modded class SCR_AmbientPatrolManager
{

	//------------------------------------------------------------------------------------------------
	override protected void ProcessSpawnpoint(int spawnpointIndex)
	{
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];
		
		SetFactionOfSpawnpoint(spawnpointIndex);

		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (!spawnpoint || spawnpoint.GetRespawnTimestamp() > Replication.Time() || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;
		#else
		if (!spawnpoint || (spawnpoint.GetMembersAlive() == 0 && !spawnpoint.GetIsSpawned()))
			return;

		ChimeraWorld world = GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if (spawnpoint.GetRespawnTimestamp().Greater(currentTime))
			return;
		#endif

#ifdef CONFLICT_SPAWN_ALL_AI
		if (!spawnpoint.GetIsSpawned())
			spawnpoint.SpawnPatrol();

		return;
#endif

		bool playersNear;
		bool playersVeryNear;
		bool playersFar = true;
		vector location = spawnpoint.GetOwner().GetOrigin();
		int distance;

		// Define if any player is close enough to spawn or if all players are far enough to despawn
		foreach (IEntity player : m_aPlayers)
		{
			if (!player)
				continue;

			distance = vector.DistanceSq(player.GetOrigin(), location);

			if (distance < m_iDespawnDistanceSq)
			{
				playersFar = false;

				if (distance < m_iSpawnDistanceSq)
				{
					playersNear = true;

					if (distance < SPAWN_RADIUS_MIN)
						playersVeryNear = true;

					break;
				}
			}
		}

		bool isAIOverLimit;
		AIWorld aiWorld = GetGame().GetAIWorld();
		
		if (aiWorld)
		{
			int maxChars = aiWorld.GetAILimit();
			
			if (maxChars <= 0)
				isAIOverLimit = true;
			else
				isAIOverLimit = ((float)aiWorld.GetCurrentAmountOfLimitedAIs() / (float)maxChars) > spawnpoint.GetAILimitThreshold();
		}

		if (!isAIOverLimit && !playersVeryNear)
			spawnpoint.SetIsPaused(false);

		if (playersNear && !spawnpoint.GetIsSpawned() && !spawnpoint.GetIsPaused())
		{
			// Do not spawn the patrol if the AI threshold setting has been reached
			if (isAIOverLimit)
			{
				spawnpoint.SetIsPaused(true);	// Make sure a patrol is not spawned too close to players when AI limit suddenly allows spawning of this group
				return;
			}

			spawnpoint.SpawnPatrol();
			return;
		}

		// Delay is used so dying players don't see the despawn happen
		/*if (spawnpoint.GetIsSpawned() && playersFar)
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			float despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == -1)
				spawnpoint.SetDespawnTimer(Replication.Time() + DESPAWN_TIMEOUT);
			else if (Replication.Time() > despawnT)
			#else
			WorldTimestamp despawnT = spawnpoint.GetDespawnTimer();

			if (despawnT == 0)
				spawnpoint.SetDespawnTimer(currentTime.PlusMilliseconds(DESPAWN_TIMEOUT));
			else if (currentTime.Greater(despawnT))
			#endif
				spawnpoint.DespawnPatrol();
		}
		else
		{
			#ifndef AR_CAMPAIGN_TIMESTAMP
			spawnpoint.SetDespawnTimer(-1);
			#else
			spawnpoint.SetDespawnTimer(null);
			#endif
		}*/
	}
	void SetFactionOfSpawnpoint(int spawnpointIndex)
	{
		bool factionupdated;
		SCR_AmbientPatrolSpawnPointComponent spawnpoint = m_aPatrols[spawnpointIndex];
		FactionAffiliationComponent affcomp = FactionAffiliationComponent.Cast(spawnpoint.GetOwner().FindComponent(FactionAffiliationComponent));
		SCR_GameModeCampaign Camp = SCR_GameModeCampaign.GetInstance();
		SCR_CampaignMilitaryBaseManager CampMan = Camp.GetBaseManager();
		array<SCR_CampaignMilitaryBaseComponent> bases = {};
		CampMan.GetBases(bases);
		if (bases.IsEmpty())
		{
			return;
		}
		float dist;
		foreach (SCR_CampaignMilitaryBaseComponent base : bases)
		{
			if (!base.GetFaction())
				continue;
			if (!dist)
			{
				dist = vector.Distance( spawnpoint.GetOwner().GetOrigin(), base.GetOwner().GetOrigin());
				affcomp.SetAffiliatedFactionByKey( base.GetFaction().GetFactionKey() );
			}
			
			float dist2 = vector.Distance( spawnpoint.GetOwner().GetOrigin(), base.GetOwner().GetOrigin());
			if ( dist2 < dist)
			{
				dist = dist2;
				affcomp.SetAffiliatedFactionByKey( base.GetFaction().GetFactionKey() );
			}
			if (dist < base.m_iBasePatrolInfluance)
			{
				factionupdated = true;
			}
		}
		if (!factionupdated)
		{
			affcomp.SetAffiliatedFactionByKey( Camp.GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetFactionKey() );
		}
	}
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}
	
}
modded enum SCR_EGroupType
{
	SOLOGUY,
};