[BaseContainerProps(configRoot:true),  CampaignBaseOverrideConfigTitleAttribute()]
class SP_CampaignBaseOverrideInfo
{
	[Attribute("Base", desc: "Name of base to override.", category: "Campaign")]
	string m_sBaseToOverride;
	
	[Attribute("", desc: "Name To Give.", category: "Campaign")]
	string m_sName;
	
	[Attribute("0" ,UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignFaction))]
	SCR_ECampaignFaction m_sFaction;
	
	[Attribute("0", category: "Campaign"), RplProp()]
	bool m_bIsControlPoint;

	[Attribute("0", desc: "Can this base be picked as a faction's main base?", category: "Campaign")]
	bool m_bCanBeHQ;
	
	[Attribute()]
	int m_iRadius;
	
	[Attribute(desc: "Description of area for event keeping purposes")]
	string m_sAreaDesc;
	
}
class CampaignBaseOverrideConfigTitleAttribute : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string BaseName;
		source.Get("m_sBaseToOverride", BaseName);
		SCR_ECampaignFaction faction;
		source.Get("m_sFaction", faction);
		title = BaseName + "| |" + typename.EnumToString(SCR_ECampaignFaction, faction);
		return true;
	}
};