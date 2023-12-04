#include <memory>

#include <array>


#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include <common/osg.h>

#include <scivis/io/tf_io.h>
#include <scivis/io/tf_osg_io.h>
#include <scivis/io/vol_io.h>
#include <scivis/io/vol_osg_io.h>
#include <scivis/scalar_viser/marching_cube_renderer.h>

#include <scivis/info_viser/scatter_plot.h>

static const std::string volPath = DATA_PATH_PREFIX"OSS/OSS000.raw";
static const std::string volDPath = "C:/Code/bin/data/vis-osg-scatter-plot/ellipse753.txt";
static const std::string volName = "0";
static const std::array<uint32_t, 3> dim = { 300, 350, 50 };
static const std::array<uint32_t, 3> graphDim = { 100, 100, 50 };
static const std::array<int32_t, 3> coordinateDimMax = { 3, 3, 2 };
static const std::array<int32_t, 3> coordinateDimMin = { -3, -3, -2 };
static const std::array<uint8_t, 3> log2Dim = { 9, 9, 6 };
static const std::array<float, 2> lonRng = { 100.05f, 129.95f };
static const std::array<float, 2> latRng = { -4.95f, 29.95 };
// static const std::array<float, 2> hRng = { 1.f, 5316.f };
static const std::array<float, 2> hRng = { 1.f, 21264.f };
static const float hScale = 100.f;

int main(int argc, char** argv)
{
	SciVis::InfoViser::ScatterPlot p;
	auto* viewer = new osgViewer::Viewer;
	viewer->setUpViewInWindow(200, 50, 800, 600);

	// ���
	auto* manipulator = new osgGA::TrackballManipulator;
	viewer->setCameraManipulator(manipulator);

	osg::ref_ptr<osg::Group> grp = new osg::Group;
	grp->addChild(createEarth());

	std::shared_ptr<SciVis::InfoViser::ScatterPlot> mcb
		= std::make_shared<SciVis::InfoViser::ScatterPlot>();
	std::string errMsg;
	{
		/*auto volU8Dat = SciVis::Loader::RAWVolume::LoadU8FromFile(volPath, dim);
		if (!errMsg.empty())
			goto ERR;
		auto volDat = SciVis::Convertor::RAWVolume::U8ToNormalizedFloat(volU8Dat);*/
		auto volDiscreteDat = SciVis::InfoViser::ScatterPlot::LoadFromTxt(volDPath);
		auto volDatShrd = std::make_shared<std::vector<osg::Vec3f>>(volDiscreteDat);
		mcb->AddDiscreteVolume(volName, volDatShrd);
		auto vol = mcb->GetVolume(volName);
		vol->SetLongtituteRange(lonRng[0], lonRng[1]);
		vol->SetLatituteRange(latRng[0], latRng[1]);
		vol->SetHeightFromCenterRange(.7f, .75f);
		vol->SetHeightFromCenterRange(
			static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) + hScale * hRng[0],
			static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) + hScale * hRng[1]);
		/*std::vector<osg::Vec3f> point = vol->GetVec(graphDim, log2Dim);
		vol->DrawPlot(point);
		grp->addChild(vol->MakeCoordinate());*/
	    std::vector<osg::Vec3f> point = vol->GetVec(graphDim, coordinateDimMax, coordinateDimMin);
		vol->DrawPlot(point);
		// grp->addChild(vol->MakeCoordinate(coordinateDimMax, coordinateDimMin));
		grp->addChild(vol->MakeCoordinate());
	}
    
    grp->addChild(mcb->GetGroup());

	viewer->setSceneData(grp);

	auto prevClk = clock();
	while (!viewer->done()) {
		auto currClk = clock();
		auto duration = currClk - prevClk;

		if (duration >= CLOCKS_PER_SEC / 45) {
			viewer->frame();
			prevClk = clock();
		}
	}

	return 0;

ERR:
	std::cerr << errMsg << std::endl;
	return 1;

}


//int main(int argc, char** argv)
//{
//	SciVis::InfoViser::ScatterPlot p;
//	auto* viewer = new osgViewer::Viewer;
//	viewer->setUpViewInWindow(200, 50, 800, 600);
//
//	// ���
//	auto* manipulator = new osgGA::TrackballManipulator;
//	viewer->setCameraManipulator(manipulator);
//
//	osg::ref_ptr<osg::Group> grp = new osg::Group;
//	grp->addChild(createEarth());
//
//	std::shared_ptr<SciVis::InfoViser::ScatterPlot> mcb
//		= std::make_shared<SciVis::InfoViser::ScatterPlot>();
//	std::string errMsg;
//	{
//		auto volU8Dat = SciVis::Loader::RAWVolume::LoadU8FromFile(volPath, dim);
//		if (!errMsg.empty())
//			goto ERR;
//		auto volDat = SciVis::Convertor::RAWVolume::U8ToNormalizedFloat(volU8Dat);
//		auto volDatShrd = std::make_shared<std::vector<float>>(volDat);
//		mcb->AddVolume(volName, volDatShrd, dim);
//		auto vol = mcb->GetVolume(volName);
//		vol->SetLongtituteRange(lonRng[0], lonRng[1]);
//		vol->SetLatituteRange(latRng[0], latRng[1]);
//		vol->SetHeightFromCenterRange(.7f, .75f);
//		vol->SetHeightFromCenterRange(
//			static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) + hScale * hRng[0],
//			static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) + hScale * hRng[1]);
//		std::vector<osg::Vec3f> point = vol->GetVec(graphDim, log2Dim);
//		vol->DrawPlot(point);
//		grp->addChild(vol->MakeCoordinate());
//	}
//    
//    grp->addChild(mcb->GetGroup());
//
//	viewer->setSceneData(grp);
//
//	auto prevClk = clock();
//	while (!viewer->done()) {
//		auto currClk = clock();
//		auto duration = currClk - prevClk;
//
//		if (duration >= CLOCKS_PER_SEC / 45) {
//			viewer->frame();
//			prevClk = clock();
//		}
//	}
//
//	return 0;
//
//ERR:
//	std::cerr << errMsg << std::endl;
//	return 1;
//
//}
