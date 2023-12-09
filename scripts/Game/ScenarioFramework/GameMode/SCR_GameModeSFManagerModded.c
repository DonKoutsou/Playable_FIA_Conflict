modded class SCR_GameModeSFManager
{
//------------------------------------------------------------------------------------------------
	override void OnTaskUpdate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (!task) 
			return;
		
		SCR_ScenarioFrameworkTask SFTask = SCR_ScenarioFrameworkTask.Cast(task);
		if (!SFTask)
			return;

		Faction faction =  task.GetTargetFaction();
		
		if (task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_LastFinishedTaskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer(); 
			m_LastFinishedTask = task;
		}
		
		if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
		{
			if (faction)
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate", faction.GetFactionKey());
			else
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate");
			
			SCR_ScenarioFrameworkLayerTask taskLayer = SFTask.GetTaskLayer();
			SCR_ScenarioFrameworkSlotTask subject = taskLayer.GetTaskSubject();
			if (subject)
				subject.OnTaskStateChanged(SCR_TaskState.UPDATED);
		}

		GetOnTaskStateChanged().Invoke(task, mask);
	}
}