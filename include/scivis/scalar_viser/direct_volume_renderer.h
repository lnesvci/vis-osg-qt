#ifndef SCIVIS_SCALAR_VISER_DIRECT_VOLUME_RENDERER_H
#define SCIVIS_SCALAR_VISER_DIRECT_VOLUME_RENDERER_H

#include <string>

#include <map>

#include <osg/CullFace>
#include <osg/CoordinateSystemNode>
#include <osg/ShapeDrawable>
#include <osg/Texture1D>
#include <osg/Texture3D>

#include <scivis/common/callback.h>

namespace SciVis
{
	namespace ScalarViser
	{

		class DirectVolumeRenderer
		{
		private:
			struct PerRendererParam
			{
				osg::ref_ptr<osg::Group> grp;
				osg::ref_ptr<osg::Program> program;

				osg::ref_ptr<osg::Uniform> eyePos;
				osg::ref_ptr<osg::Uniform> sliceCntr;
				osg::ref_ptr<osg::Uniform> sliceDir;
				osg::ref_ptr<osg::Uniform> dt;
				osg::ref_ptr<osg::Uniform> maxStepCnt;
				osg::ref_ptr<osg::Uniform> useSlice;

				class Callback : public osg::NodeCallback
				{
				private:
					osg::ref_ptr<osg::Uniform> eyePos;

				public:
					Callback(osg::ref_ptr<osg::Uniform> eyePos) : eyePos(eyePos)
					{}
					virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
					{
						auto _eyePos = nv->getEyePoint();
						eyePos->setUpdateCallback(new UniformUpdateCallback<osg::Vec3>(_eyePos));

						traverse(node, nv);
					}
				};

				PerRendererParam()
				{
					grp = new osg::Group;

					osg::ref_ptr<osg::Shader> vertShader = osg::Shader::readShaderFile(
						osg::Shader::VERTEX,
						SCIVIS_SHADER_PREFIX
						"scivis/scalar_viser/dvr_vert.glsl");
					osg::ref_ptr<osg::Shader> fragShader = osg::Shader::readShaderFile(
						osg::Shader::FRAGMENT,
						SCIVIS_SHADER_PREFIX
						"scivis/scalar_viser/dvr_frag.glsl");
					program = new osg::Program;
					program->addShader(vertShader);
					program->addShader(fragShader);

#define STATEMENT(name, val) name = new osg::Uniform(#name, val)
					STATEMENT(eyePos, osg::Vec3());
					STATEMENT(dt, static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * .008f);
					STATEMENT(maxStepCnt, 100);
					STATEMENT(useSlice, 0);
					STATEMENT(sliceCntr, osg::Vec3());
					STATEMENT(sliceDir, osg::Vec3());
#undef STATEMENT

					grp->setCullCallback(new Callback(eyePos));
				}
			};
			PerRendererParam param;

			struct PerVolParam
			{
				bool isDisplayed;

				osg::ref_ptr<osg::Uniform> minLatitute;
				osg::ref_ptr<osg::Uniform> maxLatitute;
				osg::ref_ptr<osg::Uniform> minLongtitute;
				osg::ref_ptr<osg::Uniform> maxLongtitute;
				osg::ref_ptr<osg::Uniform> minHeight;
				osg::ref_ptr<osg::Uniform> maxHeight;
				osg::ref_ptr<osg::Uniform> volStartFromZeroLon;

				osg::ref_ptr<osg::ShapeDrawable> sphere;
				osg::ref_ptr<osg::Texture3D> volTex;
				osg::ref_ptr<osg::Texture1D> tfTex;

				PerVolParam(osg::ref_ptr<osg::Texture3D> volTex, osg::ref_ptr<osg::Texture1D> tfTex,
					PerRendererParam* renderer)
					: volTex(volTex), tfTex(tfTex)
				{
					const auto MinHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.1f;
					const auto MaxHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.3f;

					auto tessl = new osg::TessellationHints;
					tessl->setDetailRatio(10.f);
					sphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.f, 0.f, 0.f), MaxHeight), tessl);

					auto states = sphere->getOrCreateStateSet();
#define STATEMENT(name, val)                                                                       \
    name = new osg::Uniform(#name, val);                                                           \
    states->addUniform(name)
					STATEMENT(minLatitute, deg2Rad(-10.f));
					STATEMENT(maxLatitute, deg2Rad(+10.f));
					STATEMENT(minLongtitute, deg2Rad(-20.f));
					STATEMENT(maxLongtitute, deg2Rad(+20.f));
					STATEMENT(minHeight, MinHeight);
					STATEMENT(maxHeight, MaxHeight);
					STATEMENT(volStartFromZeroLon, 0);
#undef STATEMENT
					states->addUniform(renderer->eyePos);
					states->addUniform(renderer->dt);
					states->addUniform(renderer->maxStepCnt);
					states->addUniform(renderer->useSlice);
					states->addUniform(renderer->sliceCntr);
					states->addUniform(renderer->sliceDir);

					states->setTextureAttributeAndModes(0, volTex, osg::StateAttribute::ON);
					states->setTextureAttributeAndModes(1, tfTex, osg::StateAttribute::ON);

					auto volTexUni = new osg::Uniform(osg::Uniform::SAMPLER_3D, "volTex");
					volTexUni->set(0);
					auto tfTexUni = new osg::Uniform(osg::Uniform::SAMPLER_1D, "tfTex");
					tfTexUni->set(1);
					states->addUniform(volTexUni);
					states->addUniform(tfTexUni);

					osg::ref_ptr<osg::CullFace> cf = new osg::CullFace(osg::CullFace::BACK);
					states->setAttributeAndModes(cf);

					states->setAttributeAndModes(renderer->program, osg::StateAttribute::ON);
					states->setMode(GL_BLEND, osg::StateAttribute::ON);
					states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
				}
				/*
				* ����: SetTransferFunction
				* ����: ���ø������ʱ�Ĵ��亯��
				* ����:
				* -- tfTex: ��Ĵ��亯����OSGһά����
				*/
				void SetTransferFunction(osg::ref_ptr<osg::Texture1D> tfTex)
				{
					this->tfTex = tfTex;
					auto states = sphere->getOrCreateStateSet();
					states->setTextureAttributeAndModes(1, this->tfTex, osg::StateAttribute::ON);
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

					minLongtitute->set(deg2Rad(minLonDeg));
					maxLongtitute->set(deg2Rad(maxLonDeg));
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

					minLatitute->set(deg2Rad(minLatDeg));
					maxLatitute->set(deg2Rad(maxLatDeg));
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

					minHeight->set(minH);
					maxHeight->set(maxH);

					sphere->setShape(new osg::Sphere(osg::Vec3(0.f, 0.f, 0.f), maxH));
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
					if (flag) volStartFromZeroLon->set(1);
					else volStartFromZeroLon->set(0);
				}

			private:
				float deg2Rad(float deg)
				{
					return deg * osg::PI / 180.f;
				};

				friend class DirectVolumeRenderer;
			};
			std::map<std::string, PerVolParam> vols;

		public:
			DirectVolumeRenderer()
			{}

			/*
			* ����: GetGroup
			* ����: ��ȡ�û��������OSG�ڵ�
			* ����ֵ: OSG�ڵ�
			*/
			osg::Group* GetGroup()
			{
				return param.grp.get();
			}
			/*
			* ����: AddVolume
			* ����: ��û���������һ����
			* ����:
			* -- name: ���������ơ���ͬ��������費ͬ����������
			* -- volTex: ���OSG��ά����
			* -- tfTex: ��Ĵ��亯����OSGһά����
			* -- isDisplayed: Ϊtrueʱ���屻�����ᱻ���ơ�������ֻ�������������������ᱻ����
			*/
			void AddVolume(const std::string& name, osg::ref_ptr<osg::Texture3D> volTex,
				osg::ref_ptr<osg::Texture1D> tfTex, bool isDisplayed = true)
			{
				auto itr = vols.find(name);
				if (itr != vols.end() && itr->second.isDisplayed) {
					param.grp->removeChild(itr->second.sphere);
					vols.erase(itr);
				}
				auto opt = vols.emplace(std::pair<std::string, PerVolParam>(name, PerVolParam(volTex, tfTex, &param)));

				opt.first->second.isDisplayed = isDisplayed;
				if (isDisplayed)
					param.grp->addChild(opt.first->second.sphere);
			}
			/*
			* ����: DisplayVolume
			* ����: ���Ƹû�������е�һ���壬λ������е������彫�������ơ�һ�����ڲ����嶯������������岻�ڸ������ʱ�������嶼���ᱻ���ơ�
			* ����:
			* -- name: �������
			*/
			void DisplayVolume(const std::string& name)
			{
				for (auto itr = vols.begin(); itr != vols.end(); ++itr) {
					if (itr->first == name) {
						itr->second.isDisplayed = true;
						param.grp->addChild(itr->second.sphere);
					}
					else if (itr->second.isDisplayed == true) {
						itr->second.isDisplayed = false;
						param.grp->removeChild(itr->second.sphere);
					}
				}
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
			/*
			* ����: SetDeltaT
			* ����: ���������ʱ�����ߴ����Ĳ���
			* ����:
			* -- dt: ���ߴ����Ĳ���
			*/
			void SetDeltaT(float dt)
			{
				param.dt->set(dt);
			}
			float GetDeltaT() const
			{
				float ret;
				param.dt->get(ret);
				return ret;
			}
			/*
			* ����: SetMaxStepCount
			* ����: ���������ʱ�����ߴ����������
			* ����:
			* -- maxStepCnt: ���ߴ����������
			*/
			void SetMaxStepCount(int maxStepCnt)
			{
				param.maxStepCnt->set(maxStepCnt);
			}
			int GetMaxStepCount() const
			{
				int ret;
				param.maxStepCnt->get(ret);
				return ret;
			}
			/*
			* ����: SetSlicing
			* ����: ����������е�����
			* ����:
			* -- cntr: ��������ĵ㣨��תʱ�Ĳο��㣩��λ����ֲ���������ȡֵ��ΧΪ[0, 1]
			* -- dir: ����ĵ�λ��������λ����ֲ�����
			*/
			void SetSlicing(const osg::Vec3& cntr, const osg::Vec3& dir)
			{
				param.useSlice->set(1);
				param.sliceCntr->set(cntr);
				param.sliceDir->set(dir);
			}
			void DisableSlicing()
			{
				param.useSlice->set(0);
			}
		};

	} // namespace ScalarViser
} // namespace SciVis

#endif // !SCIVIS_SCALAR_VISER_DIRECT_VOLUME_RENDERER_H
