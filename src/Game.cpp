#include <GL/glew.h>
#include "Game.hpp"
#include "graphical/LightManager.hpp"

using namespace Familyline;
using namespace Familyline::Logic;
using namespace Familyline::Input;

using namespace Familyline::Graphics;
using namespace Familyline::Graphics::GUI;

class GameActionListenerImpl : public GameActionListener {
public:
	GameActionListenerImpl() : GameActionListener("listener-base") {}

    virtual void OnListen(GameAction& a) {
		(void)a;
		Log::GetLog()->InfoWrite("bogus-listener", "received from listener");
    }
};


Game::Game(Window* w, Framebuffer* fb3D, Framebuffer* fbGUI,
	GUIManager* gr, PlayerManager* pm, HumanPlayer* hp)
	: win(w), fbGUI(fbGUI), fb3D(fb3D), gr(gr), pm(pm), hp(hp)
{
	DebugPlotter::Init();

	int winW, winH;
	w->GetSize(winW, winH);
	char* err = nullptr;

	try {
		/* Initialise things */
		om = new ObjectManager{};

		rndr = new Renderer{};
		DebugPlotter::pinterface = std::unique_ptr<DebugPlotInterface>
			(new GraphicalPlotInterface(rndr));

		gctx.om = om;

		gam.AddListener(new GameActionListenerImpl());
		hp->SetGameActionManager(&gam);
		pm->AddPlayer(hp, PlayerFlags::PlayerIsHuman);

		terrFile = new TerrainFile(ASSET_FILE_DIR "terrain_test.trtb");
		terr = terrFile->GetTerrain();

		tc = new TeamCoordinator();
		auto tteam = tc->CreateTeam("test");
		printf("%s -- %#x\n", tteam->name.c_str(), tteam->id);


		scenemng = new SceneManager(terr->GetWidth() * SEC_SIZE, terr->GetHeight() * SEC_SIZE);

		cam = new Camera{ glm::vec3(6.0f, 36.0f, 6.0f), (float)winW / (float)winH, glm::vec3(0,0,0) };
		scenemng->SetCamera(cam);
		hp->SetCamera(cam);

		rndr->SetSceneManager(scenemng);

		am->Create();

		Light* l = new Light{"mainLight", glm::vec3(16, 16, 16), 0x0, 0x0, 0xff, 200 };
		std::vector<glm::vec3> p = {
			glm::vec3(0, terr->GetHeightFromPoint(0, 0) + 16, 0),
			glm::vec3(10, terr->GetHeightFromPoint(10, 10) + 16, 10),
			glm::vec3(10, terr->GetHeightFromPoint(10, 0) + 16, 0)
		};

		DebugPlotter::pinterface->AddPath(p, glm::vec3(1, 0, 0));
		LightManager::AddLight(l);

		terr_rend = new TerrainRenderer{ };
		terr_rend->SetTerrain(terr);
		terr_rend->SetCamera(cam);

		objrend = new ObjectRenderer(om, scenemng);
		hp->objr = objrend;
		gam.AddListener(objrend);

		InputManager::GetInstance()->Initialize();

		ip = new InputPicker{ terr_rend, win, scenemng, cam, om };
		hp->SetPicker(ip);

		pathf = new PathFinder(om);
		pathf->InitPathmap(terr->GetWidth(), terr->GetHeight());
		pathf->UpdatePathmap(terr->GetWidth(), terr->GetHeight());
		hp->SetPathfinder(pathf);

		widgets.lblVersion = new GUILabel(10, 10, "Familyline " VERSION " commit " COMMIT);
//		widgets.lblVersion->SetForeColor(255, 255, 255, 255);

		gr->add(widgets.lblVersion);

		/* Adds the objects to the factory */
		ObjectFactory::GetInstance()->AddObject(new WatchTower);
		ObjectFactory::GetInstance()->AddObject(new Tent);

		ObjectPathManager::getInstance()->SetTerrain(terr);

	}
	catch (window_exception& we) {
		Log::GetLog()->Fatal("game", "Window creation error: %s (%d)", we.what(), we.code);
		exit(EXIT_FAILURE);
	}
	catch (renderer_exception& re) {
		Log::GetLog()->Fatal("game", "Rendering error: %s [%d]",
			re.what(), re.code);

		err = new char[192 + strlen(re.what())];
		sprintf(err,
			"Familyline found an error in rendering\n"
			"\n"
			"Error: %s\n", re.what());
		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);
		delete[] err;
		exit(EXIT_FAILURE);
	}
	catch (mesh_exception& se) {
		Log::GetLog()->Fatal("game", "Mesh error: %s", se.what());
		Log::GetLog()->Fatal("game", "Mesh file: %s", se.file.c_str());

		err = new char[512 + strlen(se.what())];
		sprintf(err,
			"Familyline found an error in a mesh\n"
			"\n"
			"Mesh file: %s, error: %s\n",
			se.file.c_str(), se.what());
		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);
		delete[] err;
		exit(EXIT_FAILURE);
	}
	catch (material_exception& se) {
		Log::GetLog()->Fatal("game", "Material error: %s ", se.what());
		Log::GetLog()->Fatal("game", "Material file: %s", se.file.c_str());

		err = new char[512 + strlen(se.what())];
		sprintf(err,
			"Familyline found an error in a material\n"
			"\n"
			"Mesh file: %s, error: %s\n",
			se.file.c_str(), se.what());
		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);
		delete[] err;
		exit(EXIT_FAILURE);
	}
	catch (shader_exception& se) {
		Log::GetLog()->Fatal("game", "Shader error: %s [%d]", se.what(), se.code);
		Log::GetLog()->Fatal("game", "Shader file: %s, type %d", se.file.c_str(), se.type);

		err = new char[512 + strlen(se.what())];
		sprintf(err,
			"Familyline found an error in a shader\n"
			"\n"
			"Error: %s\n"
			"File: %s, type: %d, code: %d",
			se.what(), se.file.c_str(), se.type, se.code);
		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);
		delete[] err;
		exit(EXIT_FAILURE);
	}
	catch (asset_exception& ae) {
		Log::GetLog()->Fatal("game", "Asset file error: %s", ae.what());
		err = new char[768 + strlen(ae.what())];

		if (ae.assetptr) {
			AssetItem* a = (AssetItem*)ae.assetptr;
			Log::GetLog()->Fatal("game", "Asset %s, file: %s", a->name.c_str(), a->path.c_str());

			sprintf(err,
				"Familyline found an error in an asset\n"
				"\n"
				"Asset named %s, file: %s\n"
				"Error: %s\n",
				a->name.c_str(), a->path.c_str(), ae.what());
		}

		sprintf(err,
			"Familyline found an error in an asset\n"
			"\n"
			"Error: %s\n",
			ae.what());

		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);
		delete[] err;
		exit(EXIT_FAILURE);
	}
	catch (terrain_file_exception& te) {
		Log::GetLog()->Fatal("game", "Terrain file error: %s on file %s", te.what(), te.file.c_str());
		if (te.code != 0) {
			Log::GetLog()->Fatal("game", "Error code: %d (%s)", te.code, strerror(te.code));
		}

		err = new char[512 + strlen(te.what())];
		sprintf(err,
			"Familyline found an error in a terrain\n"
			"\n"
			"File: %s\n"
			"error: %s (%s)\n",
			te.file.c_str(), te.what(), strerror(te.code));
		win->ShowMessageBox(err, "Error", MessageBoxInfo::Error);

		delete[] err;
		exit(EXIT_FAILURE);
	}
}

int Game::RunLoop()
{
	CombatManager::GetInstance()->SetOnDeath([&](Logic::AttackableObject* at) {
		gctx.om->UnregisterObject(at);
	});


	// Panel pnl = Panel(0.0, 0.8, 1.0, 0.2, true);
	// pnl.SetBackColor(0, 0, 0, 185);
	// gr->AddPanel(&pnl);

	// GUIActionManager* guam = new GUIActionManager(&pnl);
	// hp->SetGUIActionManager(guam);

	/*    Button btnExit = Button(0.7, 0.05, 0.2, 0.5, "Exit");
		btnExit.SetOnClickListener([&](GUI::IControl* cc) {
			(void)cc;
			player = false;
		});
		pnl.AddPanel(&btnExit); */

	GUILabel lblBuilding = GUILabel(0.05, 0.1, "!!!");
	GUILabel lblFPS = GUILabel(0.05, 0.9, "0 fps, 0 ms/frame");
	GUILabel lblRange = GUILabel(0.05, 0.15, "--");
	GUILabel lblSelected = GUILabel(0.05, 0.2, "---");
	GUILabel lblTerrainPos = GUILabel(0.05, 0.25, "---");

	GUILabel lblKeys = GUILabel(0.05, 0.05, "Press C to build Tent, E to build WatchTower, B to draw bounding boxes");

	lblBuilding.format.foreground = glm::vec4(1, 1, 1, 1);
	lblBuilding.format.background = glm::vec4(0, 0, 0, 0.4);

	lblFPS.format.foreground = glm::vec4(1, 1, 1, 1);
	lblFPS.format.background = glm::vec4(0, 0, 0, 0.4);

	lblRange.format.foreground = glm::vec4(1, 1, 1, 1);
	lblRange.format.background = glm::vec4(0, 0, 0, 0.4);

	lblSelected.format.foreground = glm::vec4(1, 1, 1, 1);
	lblSelected.format.background = glm::vec4(0, 0, 0, 0.4);

	lblKeys.format.foreground = glm::vec4(0.9, 0.8, 1, 1);
	lblKeys.format.background = glm::vec4(0, 0, 0, 0.4);

	lblTerrainPos.format.background = glm::vec4(0, 0, 0, 0.4);

	gr->add(&lblBuilding);
	gr->add(&lblFPS);
	gr->add(&lblRange);
	gr->add(&lblSelected);
	gr->add(&lblKeys);
	gr->add(&lblTerrainPos);

	unsigned int ticks = SDL_GetTicks();
	unsigned int frame = 0;

	int delta = 1;
	double pms = 0.0;

	double maxdelta = 0, mindelta = 99, sumfps = 0;
	
	do {
		/* Input processing  */
	    InputManager::GetInstance()->Run();
		ip->UpdateIntersectedObject();
		ip->UpdateTerrainProjectedPosition();

//		gr->DebugWrite(10, 40, "Press C to create an object at mouse cursor, and R to remove it.");
		player = true;
		
		gctx.elapsed_seconds = delta / 1000.0;

		InputEvent ev;
		gr->update();

		pm->ProcessInputs();
		if (!pm->PlayAll(&gctx))
			player = false;

		/*

		  Logic & graphical processing

		 */
		gam.ProcessListeners();
		terr_rend->Update();

		bool objupdate = objrend->Check();
		if (objupdate || hp->HasUpdatedObject()) {
			objrend->Update();
			pathf->UpdatePathmap(terr->GetWidth(), terr->GetHeight());
		}
		objrend->Update();

		LocatableObject* selected = hp->GetSelectedObject();

		if (BuildQueue::GetInstance()->GetNext()) {
		    char s[256];
		    sprintf(s, "Click to build %s",
			    BuildQueue::GetInstance()->GetNext()->GetName());
		    lblBuilding.setText(s);
		}
		else {
		    lblBuilding.setText("");
		}

		auto locc = ip->GetIntersectedObject();
		if (locc) {
//			gr->DebugWrite(10, 100, "Hovering object '%s'", locc->GetName());

			if (selected && locc->HasProperty("maxHP")) {
				AttackableObject* a = (AttackableObject*)locc;
				lblRange.setText( a->CheckAttackRange((AttackableObject*)selected) ? "In range" : "Not in range");
			}
		}

		{
			int qx, qy;
			scenemng->GetCameraQuadrant(qx, qy);
//			gr->DebugWrite(10, 160, "Camera quadrant: %d x %d", qx, qy);
		}


		lblSelected.setText("");
		if (selected) {
			char s[150];
			if (selected->HasProperty("maxHP")) {
				AttackableObject* a = (AttackableObject*)selected;
				sprintf(s, "Selected object: '%s' (%4d/%4d)",
						a->GetName(), (int)a->GetHP(), a->GetMaxHP());

			}
			else {
				sprintf(s, "Selected object: '%s'", selected->GetName());

			}

			lblSelected.setText(s);
		}

		CombatManager::GetInstance()->DoAttacks(gctx.elapsed_seconds);
		glm::vec3 p = ip->GetTerrainProjectedPosition();
		glm::vec2 q = ip->GetGameProjectedPosition();

		ObjectPathManager::getInstance()->UpdatePaths(delta);
//		guam->UpdateBasePanel();

		/*

		  Rendering

		 */

		fb3D->SetAsBoth();
		rndr->SetBoundingBox(hp->renderBBs);
		if (objupdate) rndr->UpdateObjects();
		rndr->UpdateFrames();
		rndr->Render(terr_rend);
		
		char texs[256];
		sprintf(texs, "Terrain pos: (OpenGL: %.3f,%.3f,%.3f | Game: %.2f, %.2f), rotation %.1f",
				p.x, p.y, p.z, q.x, q.y, cam->GetRotation() * 180 / M_PI);
		lblTerrainPos.setText(texs);
		// gr->DebugWrite(10, 65, "Bounding box: %s", hp->renderBBs ?
		// 	"Enabled" : "Disabled");

		fbGUI->SetAsBoth();
		gr->render(0, 0);
		gr->renderToScreen();
		fbGUI->Unset();

		win->Update();

		frame++;

		unsigned int elapsed = SDL_GetTicks();
		delta = elapsed - ticks;

		ticks = SDL_GetTicks();
		Timer::getInstance()->RunTimers(delta);

		if (frame % 15 == 0) {
			pms = delta * 1.0;
		}

		char sfps[128];
		sprintf(sfps, "%.3f fps, %.3f ms/frame", float(1000 / pms), float(pms));
		lblFPS.setText(sfps);
//		gr->DebugWrite(0, 420, "%.2f ms, %.2f fps", pms, 1000 / pms);

#define FPS_LOCK 120.0

		//Locked in ~120 fps
		if (delta < 1000 / FPS_LOCK) {
			auto sleepdelta = int((1000 / FPS_LOCK) - delta);
			SDL_Delay(sleepdelta);
		}

		if (delta < mindelta)
			mindelta = delta;

		if (delta > maxdelta)
			maxdelta = delta;

		sumfps += (1000 / delta);

	} while (player);

	double maxfps = 1000 / mindelta;// less delta, more fps
	double minfps = 1000 / maxdelta;
	double avgfps = sumfps / frame;
	Log::GetLog()->Write("game", "fps max: %.3f (%.3f ms), min: %.3f (%.3f ms), avg: %.3f",
		maxfps, mindelta, minfps, maxdelta, avgfps);
	Log::GetLog()->Write("game", "Total frames: %d", frame);

	return 0;
}
