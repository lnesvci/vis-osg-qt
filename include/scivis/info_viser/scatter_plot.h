#ifndef SCIVIS_INFO_VISER_SP_H
#define SCIVIS_INFO_VISER_SP_H

#include <string>

#include <array>
#include <map>
#include <unordered_map>

#include <osg/CullFace>
#include <osg/CoordinateSystemNode>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shapedrawable>

#include <osgUtil/DelaunayTriangulator>
#include <osgText/Text>
#include <osg/LineWidth>
#include <osg/StateAttribute>

#include <fstream>
#include <sstream>

namespace SciVis
{
    namespace InfoViser
    {
        class ScatterPlot
        {
		
        private:
            struct PerPlotParam
            {
                osg::ref_ptr<osg::Group> grp;
				osg::ref_ptr<osg::Program> program;
                PerPlotParam()
                {
                    grp = new osg::Group;
                    program = new osg::Program;

                }
            };
            
            PerPlotParam param;

            class PerVolParam
            {
            private:
                uint8_t rndrVertsBufIdx;
                std::array<uint32_t, 3> volDim;
				osg::Vec3 voxSz;
				float isoVal;
                float minLongtitute, maxLongtitute;
                float minLatitute, maxLatitute;
                float minHeight, maxHeight;
                bool volStartFromLonZero;

                std::shared_ptr<std::vector<float>> volDat;
				std::shared_ptr<std::vector<osg::Vec3f>> volDiscreteDat;

                osg::ref_ptr<osg::Geometry> geom;
				osg::ref_ptr<osg::Geode> geode;
				osg::ref_ptr<osg::Vec3Array> verts;
				osg::ref_ptr<osg::Vec3Array> norms;
                
            public:
                PerVolParam(decltype(volDat) volDat, const std::array<uint32_t, 3>& volDim,
					PerPlotParam* ploter)
					: volDat(volDat), volDim(volDim), rndrVertsBufIdx(0)
				{
					const auto MinHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.1f;
					const auto MaxHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.3f;

					minLongtitute = deg2Rad(-10.f);
					maxLongtitute = deg2Rad(+10.f);
					minLatitute = deg2Rad(-20.f);
					maxLatitute = deg2Rad(+20.f);
					minHeight = MinHeight;
					maxHeight = MaxHeight;
					volStartFromLonZero = false;

					voxSz = osg::Vec3(1.f / volDim[0], 1.f / volDim[1], 1.f / volDim[2]);

					verts = new osg::Vec3Array;
					norms = new osg::Vec3Array;

					geom = new osg::Geometry;
					geode = new osg::Geode;
					geode->addDrawable(geom);

					auto states = geode->getOrCreateStateSet();

					osg::ref_ptr<osg::CullFace> cf = new osg::CullFace(osg::CullFace::BACK);
					states->setAttributeAndModes(cf);

					states->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					states->setAttributeAndModes(ploter->program, osg::StateAttribute::ON);
				}
				PerVolParam(decltype(volDiscreteDat) volDiscreteDat,
					PerPlotParam* ploter)
					: volDiscreteDat(volDiscreteDat), rndrVertsBufIdx(0)
				{
					const auto MinHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.1f;
					const auto MaxHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.3f;

					minLongtitute = deg2Rad(-10.f);
					maxLongtitute = deg2Rad(+10.f);
					minLatitute = deg2Rad(-20.f);
					maxLatitute = deg2Rad(+20.f);
					minHeight = MinHeight;
					maxHeight = MaxHeight;
					volStartFromLonZero = false;

					voxSz = osg::Vec3(1.f / volDim[0], 1.f / volDim[1], 1.f / volDim[2]);

					verts = new osg::Vec3Array;
					norms = new osg::Vec3Array;

					geom = new osg::Geometry;
					geode = new osg::Geode;
					geode->addDrawable(geom);

					auto states = geode->getOrCreateStateSet();

					osg::ref_ptr<osg::CullFace> cf = new osg::CullFace(osg::CullFace::BACK);
					states->setAttributeAndModes(cf);

					states->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					states->setAttributeAndModes(ploter->program, osg::StateAttribute::ON);
				}
				/*
				* ����: SetLongtituteRange
				* ����: ���ø������ʱ�ľ��ȷ�Χ����λΪ�Ƕȣ�
				* ����:
				* -- minLonDeg: ������Сֵ
				* -- maxLonDeg: �������ֵ
				* ����ֵ: ������Ĳ������Ϸ�������false�������óɹ�������true
				*/
				bool SetLongtituteRange(float minLonDeg, float maxLonDeg)
				{
					if (minLonDeg < -180.f) return false;
					if (maxLonDeg > +180.f) return false;
					if (minLonDeg >= maxLonDeg) return false;

					minLongtitute = deg2Rad(minLonDeg);
					maxLongtitute = deg2Rad(maxLonDeg);
					return true;
				}
				/*
				* ����: SetLatituteRange
				* ����: ���ø������ʱ��γ�ȷ�Χ����λΪ�Ƕȣ�
				* ����:
				* -- minLatDeg: γ����Сֵ
				* -- maxLatDeg: γ�����ֵ
				* ����ֵ: ������Ĳ������Ϸ�������false�������óɹ�������true
				*/
				bool SetLatituteRange(float minLatDeg, float maxLatDeg)
				{
					if (minLatDeg < -90.f) return false;
					if (maxLatDeg > +90.f) return false;
					if (minLatDeg >= maxLatDeg) return false;

					minLatitute = deg2Rad(minLatDeg);
					maxLatitute = deg2Rad(maxLatDeg);
					return true;
				}
				/*
				* ����: SetHeightFromCenterRange
				* ����: ���ø������ʱ�ĸ߶ȣ������ģ���Χ
				* ����:
				* -- minH: �߶���Сֵ
				* -- maxH: �߶����ֵ
				* ����ֵ: ������Ĳ������Ϸ�������false�������óɹ�������true
				*/
				bool SetHeightFromCenterRange(float minH, float maxH)
				{
					if (minH < 0.f) return false;
					if (minH >= maxH) return false;

					minHeight = minH;
					maxHeight = maxH;
					return true;
				}
				/*
				* ����: SetVolumeStartFromLongtituteZero
				* ����: ��ȫ��������X=0��Ӧ�ľ���Ϊ0�ȣ���Ҫ�����ù���
				* ����:
				* -- flag: Ϊtrueʱ�������ù��ܡ�Ϊfalseʱ���رոù���
				*/
				void SetVolumeStartFromLongtituteZero(bool flag)
				{
					volStartFromLonZero = flag;
				}

				/*	
				* ����: DrawPlot
				* ����: ����ɢ��ͼ
				* ����:
				* -- vec: ��άɢ�����꼯��
				*/

				void DrawPlot(std::vector<osg::Vec3f>& vec)
				{	
					//��ϸ��
    				osg::TessellationHints* hints1 = new osg::TessellationHints();
    				//���þ�ϸ��
    				hints1->setDetailRatio(0.3f);

					auto vec3ToSphere = [&](const osg::Vec3& v3) -> osg::Vec3 {
						float dlt = maxLongtitute - minLongtitute;
						float x = volStartFromLonZero == 0 ? v3.x() :
							v3.x() < .5f ? v3.x() + .5f : v3.x() - .5f;
						float lon = minLongtitute + x * dlt;
						dlt = maxLatitute - minLatitute;
						float lat = minLatitute + v3.y() * dlt;
						dlt = maxHeight - minHeight;
						float h = minHeight + v3.z() * dlt;

						osg::Vec3 ret;
						ret.z() = h * sinf(lat);
						h = h * cosf(lat);
						ret.y() = h * sinf(lon);
						ret.x() = h * cosf(lon);

						return ret;
					};

					for (size_t i = 0; i < vec.size(); ++i) {
						osg::Vec3f sphereLoc = vec3ToSphere(vec[i]);
						osg::ref_ptr<osg::Sphere> pSphereShape = new osg::Sphere(sphereLoc, osg::WGS_84_RADIUS_EQUATOR / 100);
	    				osg::ref_ptr<osg::ShapeDrawable> pShapeDrawable = new osg::ShapeDrawable(pSphereShape.get(), hints1);
						pShapeDrawable->setColor(osg::Vec4(vec[i],1.0));
						geode->addDrawable(pShapeDrawable.get());
					}
					auto states = geode->getOrCreateStateSet();
					states->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					states->setMode(GL_LIGHTING, osg::StateAttribute::ON);
				}

				/*
				* ����: DrawPlot
				* ����: ����ɢ��ͼ
				*/

				void DrawPlot()
				{
					//��ϸ��
					osg::TessellationHints* hints1 = new osg::TessellationHints();
					//���þ�ϸ��
					hints1->setDetailRatio(0.3f);
					for (size_t i = 0; i < volDiscreteDat->size(); ++i) {
						osg::ref_ptr<osg::Sphere> pSphereShape = new osg::Sphere(volDiscreteDat->at(i), 0.08f);
						osg::ref_ptr<osg::ShapeDrawable> pShapeDrawable = new osg::ShapeDrawable(pSphereShape.get(), hints1);
						geode->addDrawable(pShapeDrawable.get());
					}
					auto states = geode->getOrCreateStateSet();
					states->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					states->setMode(GL_LIGHTING, osg::StateAttribute::ON);
				}

				/*
				* ����: MakeCoordinate
				* ����: ����������
				*/
				osg::Geode* MakeCoordinate(const std::array<int32_t, 3>& cooDimMax, const std::array<int32_t, 3>& cooDimMin)
				{
					// �������漸����Ϣ�Ķ���
					osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

					//�����ĸ�����
					osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
					v->push_back(osg::Vec3f(cooDimMax[0], cooDimMin[1], cooDimMin[2]));
					v->push_back(osg::Vec3f(cooDimMin[0], cooDimMin[1], cooDimMin[2]));
					v->push_back(osg::Vec3f(cooDimMin[0], cooDimMin[1], cooDimMin[2]));
					v->push_back(osg::Vec3f(cooDimMin[0], cooDimMax[1], cooDimMin[2]));
					v->push_back(osg::Vec3f(cooDimMax[0], cooDimMin[1], cooDimMin[2]));
					v->push_back(osg::Vec3f(cooDimMax[0], cooDimMin[1], cooDimMax[2]));
					geom->setVertexArray(v.get());

					osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array();
					c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
					geom->setColorArray(c.get());
					geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));
					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 2, 2));
					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 4, 2));

					osg::ref_ptr<osg::Geode> axisGeode = new osg::Geode();

					axisGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
					axisGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					axisGeode->getOrCreateStateSet()->setAttribute(new osg::LineWidth(3.0f), osg::StateAttribute::ON);

					axisGeode->addDrawable(geom.get());
					return axisGeode.release();
				}


				/*
				* ����: MakeCoordinate
				* ����: ����������
				*/
				osg::Geode* MakeCoordinate(const std::array<int32_t, 3>& cooDimMax )
				{
					// �������漸����Ϣ�Ķ���
					osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

					//�����ĸ�����
					osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();
					v->push_back(osg::Vec3f(0.0f, 0.0f, 0.0f));
					v->push_back(osg::Vec3f(cooDimMax[0], 0.0f, 0.0f));
					v->push_back(osg::Vec3f(0.0f, 0.0f, 0.0f));
					v->push_back(osg::Vec3f(0.0f, cooDimMax[1], 0.0f));
					v->push_back(osg::Vec3f(0.0f, 0.0f, 0.0f));
					v->push_back(osg::Vec3f(0.0f, 0.0f, cooDimMax[2]));
					geom->setVertexArray(v.get());

					osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array();
					c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
					c->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
					geom->setColorArray(c.get());
					geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));
					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 2, 2));
					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 4, 2));

					osg::ref_ptr<osg::Geode> axisGeode = new osg::Geode();

					axisGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
					axisGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					axisGeode->getOrCreateStateSet()->setAttribute(new osg::LineWidth(3.0f), osg::StateAttribute::ON);

					axisGeode->addDrawable(geom.get());
					return axisGeode.release();
				}

				/*
				* ����: GetVec
				* ����: �õ�ɢ�����ά������
				* ����:
				* -- eps : ���
				* -- base : ��׼ֵ
				*/
				std::vector<osg::Vec3f> GetVec(
					std::array<uint32_t, 3> graphDim, const std::array<int32_t, 3>& cooDimMax, const std::array<int32_t, 3>& cooDimMin,
					float eps = 0.011f, float base = 30.f / 255.f)
				{
					std::string errMsg;
					{
						std::vector<osg::Vec3f> ret;

						auto vec3ToSphere = [&](const osg::Vec3& v3) -> osg::Vec3 {
							float dlt = maxLongtitute - minLongtitute;
							float x = volStartFromLonZero == 0 ? v3.x() :
								v3.x() < .5f ? v3.x() + .5f : v3.x() - .5f;
							float lon = minLongtitute + x * dlt;
							dlt = maxLatitute - minLatitute;
							float lat = minLatitute + v3.y() * dlt;
							dlt = maxHeight - minHeight;
							float h = minHeight + v3.z() * dlt;

							osg::Vec3 ret;
							ret.z() = h * sinf(lat);
							h = h * cosf(lat);
							ret.y() = h * sinf(lon);
							ret.x() = h * cosf(lon);

							return ret;
						};
						
						for (size_t i = 0; i < volDiscreteDat->size(); ++i) {
							//if (volDat->at(i) < (base - eps) || volDat->at(i) > (base + eps)) continue;
							/*int z = i / volDimYxX;
							int y = (i - z * volDimYxX) / volDim[0];
							int x = i - z * volDimYxX - y * volDim[0];*/

							osg::Vec3f pointer1 = volDiscreteDat->at(i);
							osg::Vec3f volLoc = osg::Vec3f((pointer1.x() - cooDimMin[0]) * 1.0f / (cooDimMax[0] - cooDimMin[0]), (pointer1.y() - cooDimMin[1]) * 1.0f / (cooDimMax[1] - cooDimMin[1]), (pointer1.z() - cooDimMin[2]) * 1.0f / (cooDimMax[2] - cooDimMin[2]));
							//osg::Vec3f sphereLoc = vec3ToSphere(volLoc);

							ret.push_back(volLoc);
						}
						return ret;
						//std::cout << "sphere: " << ret.
					}
				ERR:
					std::cerr << errMsg << std::endl;
					return std::vector<osg::Vec3f>();
				}

				/*	
				* ����: GetVec
				* ����: �õ�ɢ�����ά������
				* ����:
				* -- eps : ���
				* -- base : ��׼ֵ
				*/
				std::vector<osg::Vec3f> GetVec(
					std::array<uint32_t, 3> graphDim, std::array<uint8_t, 3> logDstDim,
					float eps = 0.011f, float base = 30.f / 255.f)
            	{	
				
                	std::string errMsg;
                	{
                    	auto volDimYxX = static_cast<size_t>(volDim[1]) * volDim[0];
						std::vector<osg::Vec3f> ret;
						std::unordered_map<int, osg::Vec3f> idxMap;
						std::array<int, 3> dstDim = { volDim[0], volDim[1], volDim[2] };

						std::array<float, 3> scaleDst2Graph = { static_cast<float>(graphDim[0] - 1) / (dstDim[0] - 1),
										static_cast<float>(graphDim[1] - 1) / (dstDim[1] - 1),
										static_cast<float>(graphDim[2] - 1) / (dstDim[2] - 1) };
						std::array<float, 3> scaleGraph2Dst = { static_cast<float>(dstDim[0] - 1) / (graphDim[0] - 1),
										static_cast<float>(dstDim[1] - 1) / (graphDim[1] - 1),
										static_cast<float>(dstDim[2] - 1) / (graphDim[2] - 1) };

						auto vec3ToSphere = [&](const osg::Vec3& v3) -> osg::Vec3 {
							float dlt = maxLongtitute - minLongtitute;
							float x = volStartFromLonZero == 0 ? v3.x() :
								v3.x() < .5f ? v3.x() + .5f : v3.x() - .5f;
							float lon = minLongtitute + x * dlt;
							dlt = maxLatitute - minLatitute;
							float lat = minLatitute + v3.y() * dlt;
							dlt = maxHeight - minHeight;
							float h = minHeight + v3.z() * dlt;

							osg::Vec3 ret;
							ret.z() = h * sinf(lat);
							h = h * cosf(lat);
							ret.y() = h * sinf(lon);
							ret.x() = h * cosf(lon);

							return ret;
						};
						auto XYZ2Offs = [&](int x, int y, int z) {
							//return static_cast<size_t>(z) * graphDim[1] * graphDim[0] + y * graphDim[0] + x;
							return static_cast<size_t>(z) * dstDim[1] * dstDim[0] + y * dstDim[0] + x;
						};

						auto graph2Offs = [&](int x, int y, int z) {
							return static_cast<size_t>(z) * graphDim[1] * graphDim[0] + y * graphDim[0] + x;
						};

						// for (size_t i = 0; i < volDat->size(); ++i) {
						// 	if (volDat->at(i) < (base - eps) || volDat->at(i) > (base + eps) ) continue;
						// 	int z = i / volDimYxX;
						// 	int y = (i - z * volDimYxX) / volDim[0];
						// 	int x = i - z * volDimYxX - y * volDim[0];

						// 	osg::Vec3f pointer1 = osg::Vec3f(x, y, z);
						// 	osg::Vec3f volLoc = osg::Vec3f(x * voxSz.x(), y * voxSz.y(), z * voxSz.z());
						// 	osg::Vec3f sphereLoc = vec3ToSphere(volLoc);
							
						// 	ret.push_back(sphereLoc);
						// }

						int maxSub = 0;
						int maxX = 0;

						for (int z = 0; z < dstDim[2]; ++z) {
							for (int y = 0; y < dstDim[1]; ++y) {
								for (int x = 0; x < dstDim[0]; ++x) {

									auto offs = XYZ2Offs(x, y, z);
									if (volDat->at(offs) < (base - eps) || volDat->at(offs) > (base + eps) ) continue;
									maxX = std::max(maxX, x);
									auto graphZ = static_cast<int>(z * scaleDst2Graph[2]);
									auto graphY = static_cast<int>(y * scaleDst2Graph[1]);
									auto graphX = static_cast<int>(x * scaleDst2Graph[0]);
									if (graphX >= graphDim[0])
										graphX = graphDim[0] - 1;
									if (graphY >= graphDim[1])
										graphY = graphDim[1] - 1;
									if (graphZ >= graphDim[2])
										graphZ = graphDim[2] - 1;
									maxSub = std::max(maxSub, graphX);
									// osg::Vec3f volLoc = osg::Vec3f(x * voxSz.x(), y * voxSz.y(), z * voxSz.z());
									// osg::Vec3f sphereLoc = vec3ToSphere(volLoc);
									// ret.push_back(sphereLoc);

									if (graphX != 0)
										--graphX;
									if (graphY != 0)
										--graphY;
									if (graphZ != 0)
										--graphZ;
									auto graphX1 = graphX == graphDim[0] - 1 ? graphX : graphX + 1;
									auto graphY1 = graphY == graphDim[1] - 1 ? graphY : graphY + 1;
									auto graphZ1 = graphZ == graphDim[2] - 1 ? graphZ : graphZ + 1;
									

									for (int subZ = graphZ; subZ <= graphZ1; ++subZ)
										for (int subY = graphY; subY <= graphY1; ++subY)
											for (int subX = graphX; subX <= graphX1; ++subX) {

												//auto offs = XYZ2Offs(subX, subY, subZ);
												
												auto graphOffs = graph2Offs(subX, subY, subZ);
												osg::Vec3f volLoc = osg::Vec3f(subX * scaleGraph2Dst[0] * voxSz.x(), subY * scaleGraph2Dst[1] * voxSz.y(), subZ * scaleGraph2Dst[2] * voxSz.z());
												osg::Vec3f sphereLoc = vec3ToSphere(volLoc);
												if(idxMap.find(graphOffs) == idxMap.end()) {
													idxMap[graphOffs] = sphereLoc;
													ret.push_back(sphereLoc);
												}
												
											}
								}
							}
						}
						/*std::cout << "maxX: " << maxX << std::endl;
						std::cout << "maxSub: " << maxSub << std::endl;*/
						/*osg::Vec3f volLoc = osg::Vec3f((graphDim[0] - 1) * scaleGraph2Dst[0] * voxSz.x(), (graphDim[1] - 1) * scaleGraph2Dst[1] * voxSz.y(), (graphDim[2] - 1) * scaleGraph2Dst[2] * voxSz.z());
						osg::Vec3f sphereLoc = vec3ToSphere(volLoc);
						ret.push_back(sphereLoc);*/
						return ret;
                	}
            	ERR:
	            	std::cerr << errMsg << std::endl;
	            	return std::vector<osg::Vec3f>();
            	}
				/*	
				* ����: MakeCoordinate
				* ����: ����������
				*/
				osg::Geode* MakeCoordinate(const wchar_t* nameX = L"x", const wchar_t* nameY = L"y", const wchar_t* nameZ = L"z", float fontSize = osg::WGS_84_RADIUS_EQUATOR / 20)
				{	
					auto vec3ToSphere = [&](const osg::Vec3& v3) -> osg::Vec3 {
						float dlt = maxLongtitute - minLongtitute;
						float x = volStartFromLonZero == 0 ? v3.x() :
							v3.x() < .5f ? v3.x() + .5f : v3.x() - .5f;
						float lon = minLongtitute + x * dlt;
						dlt = maxLatitute - minLatitute;
						float lat = minLatitute + v3.y() * dlt;
						dlt = maxHeight - minHeight;
						float h = minHeight + v3.z() * dlt;

						osg::Vec3 ret;
						ret.z() = h * sinf(lat);
						h = h * cosf(lat);
						ret.y() = h * sinf(lon);
						ret.x() = h * cosf(lon);

						return ret;
					};

					// ���ƻ���
					osg::ref_ptr<osg::Geometry> zGeom = new osg::Geometry();
					// ԭ��

					osg::Vec3f axisOrigin = vec3ToSphere(osg::Vec3f(0.0f, 0.0f, 0.0f));
					// z��
					osg::ref_ptr<osg::Vec3Array> zVer = new osg::Vec3Array();
					zVer->push_back(axisOrigin);
					zVer->push_back(vec3ToSphere(osg::Vec3f(0.0f, 0.0f, 1.0f)));
					zGeom->setVertexArray(zVer.get());

					osg::ref_ptr<osg::Vec4Array> zColor = new osg::Vec4Array();
					zColor->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f)); 
					zColor->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f)); 
					zGeom->setColorArray(zColor.get());
					zGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
					zGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));

					// y��
					osg::ref_ptr<osg::Geometry> yGeom = new osg::Geometry();
					osg::ref_ptr<osg::Vec3Array> yVer = new osg::Vec3Array();
					for (float i = 0.0f; i <= 1.0f; i += 0.01f) {
						yVer->push_back(osg::Vec3(vec3ToSphere(osg::Vec3f(0.0f, i, 0.0f))));
					}
					yGeom->setVertexArray(yVer.get());
					osg::ref_ptr<osg::Vec4Array> yColor = new osg::Vec4Array();
					for (int i = 0; i < yVer->size(); ++i) {
						yColor->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
					}
					yGeom->setColorArray(yColor.get());
					yGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
					yGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, yVer->size()));

					// x��
					osg::ref_ptr<osg::Geometry> xGeom = new osg::Geometry();
					osg::ref_ptr<osg::Vec3Array> xVer = new osg::Vec3Array();
					for (float i = 0.0f; i <= 1.0f; i += 0.01f) {
						xVer->push_back(osg::Vec3(vec3ToSphere(osg::Vec3f(i, 0.0f, 0.0f))));
					}
					xGeom->setVertexArray(xVer.get());
					osg::ref_ptr<osg::Vec4Array> xColor = new osg::Vec4Array();
					for (int i = 0; i < xVer->size(); ++i) {
						xColor->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
					}
					xGeom->setColorArray(xColor.get());
					xGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
					xGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, xVer->size()));


					osg::ref_ptr<osg::Geode> axisGeode = new osg::Geode();
					axisGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
					axisGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
					axisGeode->getOrCreateStateSet()->setAttribute(new osg::LineWidth(3.0), osg::StateAttribute::ON);
					
					axisGeode->addDrawable(zGeom.get());
					axisGeode->addDrawable(yGeom.get());
					axisGeode->addDrawable(xGeom.get());

					// �������
					osg::ref_ptr<osgText::Text> pTextXAuxis1 = new osgText::Text;
					pTextXAuxis1->setText(nameY);

					pTextXAuxis1->setFont("Fonts/simhei.ttf");
					pTextXAuxis1->setAxisAlignment(osgText::Text::SCREEN);
					pTextXAuxis1->setCharacterSize(fontSize);
					pTextXAuxis1->setPosition(osg::Vec3(osg::Vec3(vec3ToSphere(osg::Vec3f(1.0f, 0.0f, 0.1f)))));
					pTextXAuxis1->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
					osg::ref_ptr<osgText::Text> pTextYAuxis1 = new osgText::Text;
					pTextYAuxis1->setText(nameY);
					pTextYAuxis1->setFont("Fonts/simhei.ttf");
					pTextYAuxis1->setAxisAlignment(osgText::Text::SCREEN);
					pTextYAuxis1->setCharacterSize(fontSize);
					pTextYAuxis1->setPosition(osg::Vec3(osg::Vec3(vec3ToSphere(osg::Vec3f(0.0f, 1.0f, 0.1f)))));
					pTextYAuxis1->setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
					osg::ref_ptr<osgText::Text> pTextZAuxis1 = new osgText::Text;
					pTextZAuxis1->setText(nameZ);
					pTextZAuxis1->setFont("Fonts/simhei.ttf");
					pTextZAuxis1->setAxisAlignment(osgText::Text::SCREEN);
					pTextZAuxis1->setCharacterSize(fontSize);
					pTextZAuxis1->setPosition(osg::Vec3(osg::Vec3(vec3ToSphere(osg::Vec3f(0.01f, 0.01f, 1.01f)))));
					pTextZAuxis1->setColor(osg::Vec4(0.0, 0.0, 1.0, 1.0));
					axisGeode->addDrawable(pTextXAuxis1.get());
					axisGeode->addDrawable(pTextYAuxis1.get());
					axisGeode->addDrawable(pTextZAuxis1.get());


					return axisGeode.release();
				}
            
            private:
				float deg2Rad(float deg)
				{
					return deg * osg::PI / 180.f;
				};

				friend class ScatterPlot;
			};
			std::map<std::string, PerVolParam> vols;

		public:
			ScatterPlot() {}

			
			/*
			* ����: GetGroup
			* ����: ��ȡ�û��������OSG�ڵ�
			* ����ֵ: OSG�ڵ�
			*/
			osg::Group* GetGroup() { return param.grp.get(); }
			/*
			* ����: AddVolume
			* ����: ��û���������һ����
			* ����:
			* -- name: ���������ơ���ͬ��������費ͬ����������
			* -- volDat: �����ݣ��谴Z-Y-X��˳��������
			* -- dim: �����ݵ���ά�ߴ磨XYZ˳��
			*/
			void AddVolume(const std::string& name, std::shared_ptr<std::vector<float>> volDat,
				const std::array<uint32_t, 3>& volDim)
			{
				auto itr = vols.find(name);
				if (itr != vols.end()) {
					param.grp->removeChild(itr->second.geode);
					vols.erase(itr);
				}
				vols.emplace(std::pair<std::string, PerVolParam>
					(name, PerVolParam(volDat, volDim, &param)));
				param.grp->addChild(vols.at(name).geode);
			}

			/*
			* ����: AddDiscreteVolume
			* ����: ��û���������һ����ɢ������
			* ����:
			* -- name: ���������ơ���ͬ��������費ͬ����������
			* -- volDat: �����ݣ��谴Z-Y-X��˳��������
			*/
			void AddDiscreteVolume(const std::string& name, std::shared_ptr<std::vector<osg::Vec3f>> volDiscreteDat)
			{
				auto itr = vols.find(name);
				if (itr != vols.end()) {
					param.grp->removeChild(itr->second.geode);
					vols.erase(itr);
				}
				vols.emplace(std::pair<std::string, PerVolParam>
					(name, PerVolParam(volDiscreteDat, &param)));
				param.grp->addChild(vols.at(name).geode);
			}
			/*
			* ����: GetVolumes
			* ����: ��ȡ������У����ڻ���ʱ�������������
			*/
			std::map<std::string, PerVolParam>& GetVolumes()
			{
				return vols;
			}
			/*
			* ����: GetVolume
			* ����: ��ȡ������У����ڻ���ʱ���������
			* ����:
			* -- name: �������
			* ����ֵ: ��Ļ�������
			*/
			PerVolParam* GetVolume(const std::string& name)
			{
				auto itr = vols.find(name);
				if (itr == vols.end())
					return nullptr;
				return &(itr->second);
			}
			/*
			* ����: GetVolumeNum
			* ����: ��ȡ�û���������������
			* ����ֵ: �������
			*/
			size_t GetVolumeNum() const
			{
				return vols.size();
			}

			/* ��ȡtxt�����ļ� */
			static std::vector<osg::Vec3f> LoadFromTxt(const std::string& filePath, std::string* errMsg = nullptr)
			{
				std::ifstream is(filePath);
				if (!is.is_open()) {
					if (errMsg) 
						*errMsg = "Invalid File Path";
					return std::vector<osg::Vec3f>();
				}

				std::string line;
				std::vector<osg::Vec3f> ret;
				int flag = 0;
				while (std::getline(is, line)) {
					if (flag == 0) {
						flag++;
						continue;
					}
					float x, y, z;
					std::stringstream ss(line);
					ss >> x;
					ss >> y;
					ss >> z;
					osg::Vec3f point(x, y, z);
					ret.push_back(point);
					flag--;
				}
				return ret;

			}

            
            
        }; // class ScatterPlot
    } // namespace InfoViser
} // namespace Scivis


#endif