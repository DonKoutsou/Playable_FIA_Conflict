
modded class SCR_CampaignMilitaryBaseManager
{
	override void ProcessRemnantsPresence()
	{
		SCR_AmbientPatrolManager manager = SCR_AmbientPatrolManager.GetInstance();

		if (!manager)
			return;

		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);

		int distLimit = Math.Pow(PARENT_BASE_DISTANCE_THRESHOLD, 2);
		float minDistance;
		SCR_CampaignMilitaryBaseComponent nearestBase;
		bool register = true;
		float dist;
		vector center;

		int distLimitHQ = Math.Pow(HQ_NO_REMNANTS_RADIUS, 2);
		int distLimitHQPatrol = Math.Pow(HQ_NO_REMNANTS_PATROL_RADIUS, 2);

		foreach (SCR_AmbientPatrolSpawnPointComponent patrol : patrols)
		{
			minDistance = float.MAX;
			register = true;
			center = patrol.GetOwner().GetOrigin();
			nearestBase = null;

			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized() || base.GetType() == SCR_ECampaignBaseType.RELAY)
					continue;

				dist = vector.DistanceSqXZ(center, base.GetOwner().GetOrigin());

				// Don't clear Remnants patrols around HQs if their state was already loaded
				/*if (base.IsHQ() && !m_Campaign.WasRemnantsStateLoaded())
				{
					if (dist < distLimitHQ)
					{
						patrol.SetMembersAlive(0);
						register = false;
						break;

					}
					else if (dist < distLimitHQPatrol)
					{
						AIWaypointCycle waypoint = AIWaypointCycle.Cast(patrol.GetWaypoint());

						if (waypoint)
						{
							patrol.SetMembersAlive(0);
							register = false;
							break;
						}
					}
				}*/

				if (dist > distLimit || dist > minDistance)
					continue;

				if (!base.IsHQ())
				{
					nearestBase = base;
					minDistance = dist;
				}
				else
				{
					register = false;
					break;
				}
			}

			if (register && nearestBase)
				nearestBase.RegisterRemnants(patrol);
		}
	}
	//------------------------------------------------------------------------------------------------
	int GetBases(out array<SCR_CampaignMilitaryBaseComponent> bases)
	{
		if (bases)
			return bases.Copy(m_aBases);
		else
			return m_aBases.Count();
	}
	override void OnAllBasesInitialized()
	{
		m_bAllBasesInitialized = true;

		// On server, this is done in gamemode class Start() method
		if (m_Campaign.IsProxy())
			UpdateBases();

		if (m_OnAllBasesInitialized)
			m_OnAllBasesInitialized.Invoke();

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			//InitializeSupplyDepotIcons();
			HideUnusedBaseIcons();
		}

		if (m_Campaign.IsProxy())
			return;

		SCR_MilitaryBaseManager.GetInstance().GetOnLogicRegisteredInBase().Insert(DisableExtraSeizingComponents);
		ProcessRemnantsPresence();
		SCR_CampaignFactionManager factman = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		array < Faction > factions = {};
		factman.GetFactionsList(factions);
		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction cfaction = SCR_CampaignFaction.Cast(faction);
			RecalculateRadioCoverage(cfaction);
		}
		//RecalculateRadioConverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		//RecalculateRadioConverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		//RecalculateRadioConverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
	}
	
	override void SetHQFactions(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		/*SCR_CampaignFaction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignFaction factionINDFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR);

		if (selectedHQs[0].GetFaction() == selectedHQs[1].GetFaction())
		{
			// Preset owners are the same or null, assign new owners normally
			selectedHQs[0].SetFaction(factionBLUFOR);
			selectedHQs[1].SetFaction(factionOPFOR);
			selectedHQs[2].SetFaction(factionINDFOR);
		}
		else
		{
			// Check if one of the preset owners is invalid, if yes, assign a new owner which is not assigned to the other HQ
			if (!selectedHQs[0].GetFaction() || selectedHQs[0].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[1].GetFaction() == factionBLUFOR)
					selectedHQs[0].SetFaction(factionOPFOR);
				else
					selectedHQs[0].SetFaction(factionBLUFOR);
				
				selectedHQs[2].SetFaction(factionINDFOR);
			}
			else if (!selectedHQs[1].GetFaction() || selectedHQs[1].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[0].GetFaction() == factionBLUFOR)
					selectedHQs[1].SetFaction(factionOPFOR);
				else
					selectedHQs[1].SetFaction(factionBLUFOR);
				
				selectedHQs[2].SetFaction(factionINDFOR);
			}
		}*/
	};
	//------------------------------------------------------------------------------------------------
	//! Picks Main Operating Bases from a list of candidates by checking average distance to active control points
	override void SelectHQs(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		// Pick the same HQs every time when debugging
#ifdef TDM_CLI_SELECTION
		SelectHQsSimple(candidates, selectedHQs);
		return;
#endif

		int candidatesCount = candidates.Count();

		// If only two HQs are set up, don't waste time with processing
		if (candidatesCount == 2)
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}
		
		SCR_CampaignFactionManager factman = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		array < Faction > factions = {};
		factman.GetFactionsList(factions);
		
		array <SCR_CampaignMilitaryBaseComponent> factionbases = {};
		foreach (Faction faction : factions)
		{
			
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ.GetFaction() == faction)
				{
					selectedHQs.Insert(otherHQ);
				}
			}
		}
		return;
		
		int iterations;
		int totalBasesDistance;
		SCR_CampaignMilitaryBaseComponent bluforHQ;
		SCR_CampaignFaction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignMilitaryBaseComponent opforHQ;
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignMilitaryBaseComponent inforHQ;
		SCR_CampaignFaction factionINDFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR);
		array<SCR_CampaignMilitaryBaseComponent> eligibleForHQ;
		vector bluforHQPos;
		int averageHQDistance;
		int averageCPDistance;
		int acceptedCPDistanceDiff;
		
		while (!opforHQ && iterations < MAX_HQ_SELECTION_ITERATIONS)
		{
			iterations++;
			totalBasesDistance = 0;
			eligibleForHQ = {};

			// Pick one of the HQs at random
			Math.Randomize(-1);
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ.GetFaction() == factionBLUFOR)
				{
					bluforHQ = otherHQ;
					break;
				}
			}
			//bluforHQ = candidates.GetRandomElement();
			bluforHQPos = bluforHQ.GetOwner().GetOrigin();

			// Calculate average distance between our HQ and others
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ == bluforHQ)
					continue;

				totalBasesDistance += vector.DistanceSqXZ(bluforHQPos, otherHQ.GetOwner().GetOrigin());
			}

			averageHQDistance = totalBasesDistance / (candidatesCount - 1);	// Our HQ is subtracted
			averageCPDistance = GetAvgCPDistanceSq(bluforHQ, controlPoints);
			acceptedCPDistanceDiff = averageCPDistance * CP_AVG_DISTANCE_TOLERANCE;

			foreach (SCR_CampaignMilitaryBaseComponent candidate : candidates)
			{
				if (candidate == bluforHQ)
					continue;

				// Ignore HQs closer than the average distance
				if (vector.DistanceSqXZ(bluforHQPos, candidate.GetOwner().GetOrigin()) < averageHQDistance)
					continue;

				// Ignore HQs too far from control points (relative to our HQ)
				if (Math.AbsInt(averageCPDistance - GetAvgCPDistanceSq(candidate, controlPoints)) > acceptedCPDistanceDiff)
					continue;

				eligibleForHQ.Insert(candidate);
			}

			// No HQs fit the condition, restart loop
			if (eligibleForHQ.Count() == 0)
				continue;

			Math.Randomize(-1);
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ.GetFaction() == factionOPFOR)
				{
					opforHQ = otherHQ;
					break;
				}
			}
			//opforHQ = eligibleForHQ.GetRandomElement();
		}
		if (!inforHQ)
		{
			foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
			{
				if (otherHQ.GetFaction() == factionINDFOR)
				{
					inforHQ = otherHQ;
					break;
				}
			}
		}

		// Selection failed, use the simplified but reliable one
		if (!opforHQ)
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}

		// Randomly assign the factions in reverse in case primary selection gets too limited
		if (Math.RandomFloat01() >= 0.5)
			selectedHQs = {bluforHQ, opforHQ, inforHQ};
		else
			selectedHQs = {opforHQ, bluforHQ, inforHQ};
	}
	override void InitializeBases(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs, bool randomizeSupplies)
	{
		array<SCR_CampaignMilitaryBaseComponent> basesSorted = {};
		SCR_CampaignMilitaryBaseComponent baseCheckedAgainst;
		vector originHQ1 = selectedHQs[0].GetOwner().GetOrigin();
		vector originHQ2 = selectedHQs[1].GetOwner().GetOrigin();
		float distanceToHQ;
		bool indexFound;
		int callsignIndex;
		array<int> allCallsignIndexes = {};
		array<int> callsignIndexesBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBaseCallsignIndexes();
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		// Grab all valid base callsign indexes (if both factions have the index)
		foreach (int indexBLUFOR : callsignIndexesBLUFOR)
		{
			if (factionOPFOR.GetBaseCallsignByIndex(indexBLUFOR))
				allCallsignIndexes.Insert(indexBLUFOR);
		}

		int callsignsCount = allCallsignIndexes.Count();
		Math.Randomize(-1);
		Faction defaultFaction;
		BaseRadioComponent radio;
		BaseTransceiver tsv;

		foreach (int iBase, SCR_CampaignMilitaryBaseComponent campaignBase : m_aBases)
		{
			if (!campaignBase.IsInitialized())
				continue;

			defaultFaction = campaignBase.GetFaction(true);

			// Apply default faction set in FactionAffiliationComponent or INDFOR if undefined
			if (!campaignBase.GetFaction())
			{
				if (defaultFaction)
					campaignBase.SetFaction(defaultFaction);
				else
					campaignBase.SetFaction(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
			}

			// Register bases in range of each other
			campaignBase.UpdateBasesInRadioRange();

			// Assign callsign
			if (campaignBase.GetType() == SCR_ECampaignBaseType.BASE && !allCallsignIndexes.IsEmpty())
			{
				callsignIndex = allCallsignIndexes.GetRandomIndex();
				campaignBase.SetCallsignIndex(allCallsignIndexes[callsignIndex]);
				allCallsignIndexes.Remove(callsignIndex);
			}
			else
			{
				// Relays use a dummy callsign just so search by callsign is still possible
				campaignBase.SetCallsignIndex(callsignsCount + iBase);
			}

			// Sort bases by distance to a HQ so randomized supplies can be applied fairly (if enabled)
			if (randomizeSupplies && campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
			{
				indexFound = false;
				distanceToHQ = vector.DistanceSqXZ(originHQ1, campaignBase.GetOwner().GetOrigin());

				for (int i = 0, count = basesSorted.Count(); i < count; i++)
				{
					baseCheckedAgainst = basesSorted[i];

					if (distanceToHQ < vector.DistanceSqXZ(originHQ1, baseCheckedAgainst.GetOwner().GetOrigin()))
					{
						basesSorted.InsertAt(campaignBase, i);
						indexFound = true;
						break;
					}
				}

				if (!indexFound)
					basesSorted.Insert(campaignBase);
			}
			if (!randomizeSupplies && campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
			{
				campaignBase.SetStartingSupplies(campaignBase.GetStartingSupplies() );
			}
		}

		if (randomizeSupplies)
			AddRandomSupplies(basesSorted, selectedHQs);
	}
	//------------------------------------------------------------------------------------------------
	//! Add randomized supplies to each base, calculate batches so each side encounters similarly stacked bases
 	override void AddRandomSupplies(notnull array<SCR_CampaignMilitaryBaseComponent> basesSorted, notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		array<int> suppliesBufferBLUFOR = {};
		array<int> suppliesBufferOPFOR = {};
		array<int> suppliesBufferINFOR = {};
		int intervalMultiplier = Math.Floor((m_Campaign.GetMaxStartingSupplies() - m_Campaign.GetMinStartingSupplies()) / m_Campaign.GetStartingSuppliesInterval());
		FactionKey factionToProcess;
		vector basePosition;
		float distanceToHQ1;
		float distanceToHQ2;
		float distanceToHQ3;
		int suppliesToAdd;

		foreach (SCR_CampaignMilitaryBaseComponent base : basesSorted)
		{
			if (base.IsHQ())
				continue;

			basePosition = base.GetOwner().GetOrigin();
			distanceToHQ1 = vector.DistanceSq(basePosition, selectedHQs[0].GetOwner().GetOrigin());
			distanceToHQ2 = vector.DistanceSq(basePosition, selectedHQs[1].GetOwner().GetOrigin());
			distanceToHQ3 = vector.DistanceSq(basePosition, selectedHQs[2].GetOwner().GetOrigin());
			
			if (distanceToHQ2 < distanceToHQ1 && distanceToHQ2 < distanceToHQ3)
				factionToProcess = selectedHQs[1].GetCampaignFaction().GetFactionKey();
			else if (distanceToHQ1 < distanceToHQ2 && distanceToHQ1 < distanceToHQ3)
				factionToProcess = selectedHQs[0].GetCampaignFaction().GetFactionKey();
			else if (distanceToHQ3 < distanceToHQ1 && distanceToHQ3 < distanceToHQ2)
				factionToProcess = selectedHQs[2].GetCampaignFaction().GetFactionKey();

			// Check if we have preset supplies stored in buffer
			if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR) && !suppliesBufferBLUFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferBLUFOR[0];
				suppliesBufferBLUFOR.RemoveOrdered(0);
			}
			else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR) && !suppliesBufferOPFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferOPFOR[0];
				suppliesBufferOPFOR.RemoveOrdered(0);
			}
			else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR) && !suppliesBufferINFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferINFOR[0];
				suppliesBufferINFOR.RemoveOrdered(0);
			}
			else
			{
				// Supplies from buffer not applied, add random amount, store to opposite faction's buffer
				suppliesToAdd = m_Campaign.GetMinStartingSupplies() + (m_Campaign.GetStartingSuppliesInterval() * Math.RandomIntInclusive(0, intervalMultiplier));
				
				if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
					suppliesBufferOPFOR.Insert(suppliesToAdd);
				else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR))
					suppliesBufferINFOR.Insert(suppliesToAdd);
				else
					suppliesBufferBLUFOR.Insert(suppliesToAdd);
				
			}

			base.SetStartingSupplies(suppliesToAdd);
		}
	}
	
}

