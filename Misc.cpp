#include "Misc.h"
#include "Interfaces.h"
#include "Render.h"
#include <time.h>
#include "BaseClient.h"
#include "Strafer.h"
#include <chrono>

std::string animatedclantag;
int iLastTime;
bool bDone = false;
void misc::OnCreateMove(CInput::CUserCmd *cmd, C_BaseEntity *local)
{


	if (g_Options.Misc.Bhop && local->IsAlive())
	{
		if (cmd->buttons & IN_JUMP && !(local->GetMoveType() & MOVETYPE_LADDER))
		{

			int iFlags = local->GetFlags();
			if (!(iFlags & FL_ONGROUND))
				cmd->buttons &= ~IN_JUMP;
		}
	}
	if (g_Options.Misc.syncclantag)
	{
		static int counter = 0;
		static int motion = 0;
		int ServerTime = (float)local->GetTickBase() * g_Globals->interval_per_tick * 2.5;

		if (counter % 48 == 0)
			motion++;
		int value = ServerTime % 20;
		switch (value) {
		case 0: setclantag(XorStr("heus.         P")); break;
		case 1: setclantag(XorStr("eus.         Pr")); break;
		case 2: setclantag(XorStr("us.         Pro")); break;
		case 3: setclantag(XorStr("s.         Prom")); break;
		case 4: setclantag(XorStr(".         Prome")); break;
		case 5: setclantag(XorStr("         Promet")); break;
		case 6: setclantag(XorStr("        Prometh")); break;
		case 7: setclantag(XorStr("       Promethe")); break;
		case 8: setclantag(XorStr("      Prometheu")); break;
		case 9: setclantag(XorStr("     Prometheus")); break;
		case 10: setclantag(XorStr("    Prometheus.")); break;
		case 11: setclantag(XorStr("   Prometheus. ")); break;
		case 12: setclantag(XorStr("  Prometheus.  ")); break;
		case 13: setclantag(XorStr(" Prometheus.   ")); break;
		case 14: setclantag(XorStr("Prometheus.    ")); break;
		case 15: setclantag(XorStr("rometheus.     ")); break;
		case 16: setclantag(XorStr("ometheus.      ")); break;
		case 17: setclantag(XorStr("metheus.       ")); break;
		case 18: setclantag(XorStr("etheus.        ")); break;
		case 19: setclantag(XorStr("theus.         ")); break;

		}
		counter++;
	}
	if (g_Options.Misc.namePrometheus)
	{
		char name[151];
		char fucked_char = static_cast<char>(-1);
		for (auto i = 0; i <= 150; i++)
			name[i] = fucked_char;

		const char nick[12] = "Prometheus";
		memcpy(name, nick, 11);

		SetName(name);
	}
	
	ChatSpammer();
	blockbot(cmd);
	logsrun();
}


void misc::logsrun()
{

	static auto developer = g_CVar->FindVar("developer");
	developer->SetValue(1);
	static auto con_filter_text = g_CVar->FindVar("con_filter_text");
	static auto con_filter_text_out = g_CVar->FindVar("con_filter_text_out");
	static auto con_filter_enable = g_CVar->FindVar("con_filter_enable");

	con_filter_text->SetValue("hit ");
	con_filter_text_out->SetValue("");
	con_filter_enable->SetValue(2);
}



void misc::blockbot(CInput::CUserCmd* m_pcmd)
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());


	auto m_local = pLocal;
	float bestdist = 250.f;
	int index = -1;

	if (GetAsyncKeyState(g_Options.Misc.iBlockbotKey) && g_Options.Misc.bBlockbot)
	{
		for (int i = 0; i < g_Globals->maxClients; i++) {
			auto entity = g_EntityList->GetClientEntity(i);

			if (!entity) continue;
			if (!entity->IsAlive() || entity->IsDormant() || entity == m_local) continue;

			float dist = (m_local->GetOrigin() - entity->GetOrigin()).Length();

			if (dist < bestdist) {
				bestdist = dist;
				index = i;
			}
		}

	}
	if (index == -1) return;


	auto target = g_EntityList->GetClientEntity(index);


	if (!target) return;


	Vector angles; 
	CalcAngle(m_local->GetEyePosition(), target->GetEyePosition(), angles);
	angles.Normalized();


	if (angles.y < 0.0f) m_pcmd->sidemove = 450.f;
	else if (angles.y > 0.0f) m_pcmd->sidemove = -450.f;

}


std::vector<std::string> msgs =
{
	"Prometheus owns me and all"
};

std::vector<std::string> msgs2 =
{
	"www.AIMWARE.net | CSGO Premimum Cheat"
};
void misc::ChatSpammer()
{
	static DWORD lastspammed = 0;

	if (!g_Engine->IsInGame() || !g_Engine->IsConnected() || !g_Options.Misc.spammeron)
		return;



		switch(g_Options.Misc.spammer)
		{
			
		case 0: break;

		case 1:
			if (GetTickCount() - lastspammed > 800)
			{
				lastspammed = GetTickCount();
				auto say = "say ";
				std::string p = say + msgs[rand() % msgs.size()];
				g_Engine->ClientCmd_Unrestricted(p.c_str());
			}
			break;

		case 2:
			if (GetTickCount() - lastspammed > 800)
			{
				lastspammed = GetTickCount();
				auto say = "say ";
				std::string p = say + msgs2[rand() % msgs2.size()];
				g_Engine->ClientCmd_Unrestricted(p.c_str());
			}
			break;
		}
}


static vec_t Normalize_y(vec_t ang)
{
	while (ang < -180.0f)
		ang += 360.0f;
	while (ang > 180.0f)
		ang -= 360.0f;
	return ang;
}

void misc::AutoStrafe(CInput::CUserCmd *cmd, C_BaseEntity *local, QAngle oldangles)
{
	static AutoStrafer Strafer;

	static float move = 450;
	float s_move = move * 0.5065f;
	if (local->GetMoveType() & (MOVETYPE_NOCLIP | MOVETYPE_LADDER))
		return;
	if (cmd->buttons & (IN_FORWARD | IN_MOVERIGHT | IN_MOVELEFT | IN_BACK))
		return;

	if (cmd->buttons & IN_JUMP || !(local->GetFlags() & FL_ONGROUND))
	{
		if (local->GetVelocity().Length2D() == 0 && (cmd->forwardmove == 0 && cmd->sidemove == 0))
		{
			cmd->forwardmove = 450.f;
		}
		else if (cmd->forwardmove == 0 && cmd->sidemove == 0)
		{
			if (cmd->mousedx > 0 || cmd->mousedx > -0)
			{
				cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
			}
			else
			{
				auto airaccel = g_CVar->FindVar("sv_airaccelerate");
				auto maxspeed = g_CVar->FindVar("sv_maxspeed");

				static int zhop = 0;
				double yawrad = Normalize_y(oldangles.y) * PI / 180;

				float speed = maxspeed->GetFloat();
				if (cmd->buttons & IN_DUCK)
					speed *= 0.333;

				double tau = g_Globals->interval_per_tick, MA = speed * airaccel->GetFloat();

				int Sdir = 0, Fdir = 0;
				Vector velocity = local->GetVelocity();
				double vel[3] = { velocity[0], velocity[1], velocity[2] };
				double pos[2] = { 0, 0 };
				double dir[2] = { std::cos((oldangles[1] + 10 * zhop) * PI / 180), std::sin((oldangles[1] + 10 * zhop) * PI / 180) };
				oldangles.y = Normalize_y(yawrad * 180 / PI);
				Strafer.strafe_line_opt(yawrad, Sdir, Fdir, vel, pos, 30.0, tau, MA, pos, dir);
				cmd->sidemove = Sdir * 450;
			}
		}

	}
	movementfix(oldangles, cmd);
}