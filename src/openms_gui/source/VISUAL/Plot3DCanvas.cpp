// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2021.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg$
// $Authors: Marc Sturm $
// --------------------------------------------------------------------------

//OpenMS
#include <OpenMS/FORMAT/FileHandler.h>
#include <OpenMS/SYSTEM/FileWatcher.h>
#include <OpenMS/VISUAL/ColorSelector.h>
#include <OpenMS/VISUAL/DIALOGS/Plot3DPrefDialog.h>
#include <OpenMS/VISUAL/MISC/GUIHelpers.h>
#include <OpenMS/VISUAL/MultiGradientSelector.h>
#include <OpenMS/VISUAL/Plot3DCanvas.h>
#include <OpenMS/VISUAL/Plot3DOpenGLCanvas.h>
#include <OpenMS/VISUAL/PlotWidget.h>

#include <QResizeEvent>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

using namespace std;

namespace OpenMS
{
  using namespace Internal;

  Plot3DCanvas::Plot3DCanvas(const Param & preferences, QWidget * parent) :
    PlotCanvas(preferences, parent)
  {
    // Parameter handling
    defaults_.setValue("dot:shade_mode", 1, "Shade mode: single-color ('flat') or gradient peaks ('smooth').");
    defaults_.setMinInt("dot:shade_mode", 0);
    defaults_.setMaxInt("dot:shade_mode", 1);
    defaults_.setValue("dot:gradient", "Linear|0,#ffea00;6,#ff0000;14,#aa00ff;23,#5500ff;100,#000000", "Peak color gradient.");
    defaults_.setValue("dot:interpolation_steps", 1000, "Interpolation steps for peak color gradient precalculation.");
    defaults_.setMinInt("dot:interpolation_steps", 1);
    defaults_.setMaxInt("dot:interpolation_steps", 1000);
    defaults_.setValue("dot:line_width", 2, "Line width for peaks.");
    defaults_.setMinInt("dot:line_width", 1);
    defaults_.setMaxInt("dot:line_width", 99);
    defaults_.setValue("background_color", "#ffffff", "Background color");
    setName("Plot3DCanvas");
    defaultsToParam_();
    setParameters(preferences);

    linear_gradient_.fromString(param_.getValue("dot:gradient"));
    openglcanvas_ = new Plot3DOpenGLCanvas(this, *this);
    setFocusProxy(openglcanvas_);
    connect(this, SIGNAL(actionModeChange()), openglcanvas_, SLOT(actionModeChange()));
    legend_shown_ = true;

    //connect preferences change to the right slot
    connect(this, SIGNAL(preferencesChange()), this, SLOT(currentLayerParamtersChanged_()));
  }

  Plot3DCanvas::~Plot3DCanvas()
  {
  }

  void Plot3DCanvas::resizeEvent(QResizeEvent * e)
  {
    openglcanvas_->resize(e->size().width(), e->size().height());
  }

  void Plot3DCanvas::showLegend(bool show)
  {
    legend_shown_ = show;
    update_(OPENMS_PRETTY_FUNCTION);
  }

  bool Plot3DCanvas::isLegendShown() const
  {
    return legend_shown_;
  }

  bool Plot3DCanvas::finishAdding_()
  {
    if (layers_.getCurrentLayer().type != LayerDataBase::DT_PEAK)
    {
      popIncompleteLayer_("This widget supports peak data only. Aborting!");
      return false;
    }

    //Abort if no data points are contained
    if (getCurrentLayer().getPeakData()->empty())
    {
      popIncompleteLayer_("Cannot add a dataset that contains no survey scans. Aborting!");
      return false;
    }

    recalculateRanges_(0, 1, 2);
    resetZoom(false);

    //Warn if negative intensities are contained
    if (getCurrentMinIntensity() < 0.0)
    {
      QMessageBox::warning(this, "Warning", "This dataset contains negative intensities. Use it at your own risk!");
    }

    emit layerActivated(this);
    openglwidget()->recalculateDotGradient_(getCurrentLayer());
    update_buffer_ = true;
    update_(OPENMS_PRETTY_FUNCTION);

    return true;
  }

  void Plot3DCanvas::activateLayer(Size index)
  {
    layers_.setCurrentLayer(index);
    emit layerActivated(this);
    update_(OPENMS_PRETTY_FUNCTION);
  }

  void Plot3DCanvas::removeLayer(Size layer_index)
  {
    if (layer_index >= getLayerCount())
    {
      return;
    }

    layers_.removeLayer(layer_index);

    recalculateRanges_(0, 1, 2);
    if (layers_.empty())
    {
      overall_data_range_ = DRange<3>::empty;
      update_buffer_ = true;
      update_(OPENMS_PRETTY_FUNCTION);
      return;
    }

    resetZoom();
  }

  Plot3DOpenGLCanvas * Plot3DCanvas::openglwidget() const
  {
    return static_cast<Plot3DOpenGLCanvas *>(openglcanvas_);
  }

#ifdef DEBUG_TOPPVIEW
  void Plot3DCanvas::update_(const char * caller)
  {
    cout << "BEGIN " << OPENMS_PRETTY_FUNCTION << " caller: " << caller << endl;
#else
  void Plot3DCanvas::update_(const char * /* caller */)
  {
#endif

    // make sure OpenGL already properly initialized
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx || !ctx->isValid()) return;
    
    if (update_buffer_)
    {
      update_buffer_ = false;
      if (intensity_mode_ == PlotCanvas::IM_SNAP)
      {
        openglwidget()->updateIntensityScale();
      }
      openglwidget()->initializeGL();
    }
    openglwidget()->resizeGL(width(), height());
    openglwidget()->repaint();
  }

  void Plot3DCanvas::showCurrentLayerPreferences()
  {
    Internal::Plot3DPrefDialog dlg(this);
    LayerDataBase& layer = getCurrentLayer();

// cout << "IN: " << param_ << endl;

    ColorSelector * bg_color = dlg.findChild<ColorSelector *>("bg_color");
    QComboBox * shade = dlg.findChild<QComboBox *>("shade");
    MultiGradientSelector * gradient = dlg.findChild<MultiGradientSelector *>("gradient");
    QSpinBox * width  = dlg.findChild<QSpinBox *>("width");

    bg_color->setColor(QColor(String(param_.getValue("background_color").toString()).toQString()));
    shade->setCurrentIndex(layer.param.getValue("dot:shade_mode"));
    gradient->gradient().fromString(layer.param.getValue("dot:gradient"));
    width->setValue(UInt(layer.param.getValue("dot:line_width")));

    if (dlg.exec())
    {
      param_.setValue("background_color", bg_color->getColor().name().toStdString());
      layer.param.setValue("dot:shade_mode", shade->currentIndex());
      layer.param.setValue("dot:gradient", gradient->gradient().toString());
      layer.param.setValue("dot:line_width", width->value());

      emit preferencesChange();
    }
  }

  void Plot3DCanvas::currentLayerParamtersChanged_()
  {
    openglwidget()->recalculateDotGradient_(layers_.getCurrentLayer());
    recalculateRanges_(0, 1, 2);

    update_buffer_ = true;
    update_(OPENMS_PRETTY_FUNCTION);
  }

  void Plot3DCanvas::contextMenuEvent(QContextMenuEvent * e)
  {
    //Abort of there are no layers
    if (layers_.empty())
    {
      return;
    }
    QMenu * context_menu = new QMenu(this);
    QAction * result = nullptr;

    //Display name and warn if current layer invisible
    String layer_name = String("Layer: ") + getCurrentLayer().getName();
    if (!getCurrentLayer().visible)
    {
      layer_name += " (invisible)";
    }
    context_menu->addAction(layer_name.toQString())->setEnabled(false);
    context_menu->addSeparator();
    context_menu->addAction("Layer meta data");

    QMenu * save_menu = new QMenu("Save");
    context_menu->addMenu(save_menu);
    save_menu->addAction("Layer");
    save_menu->addAction("Visible layer data");

    QMenu * settings_menu = new QMenu("Settings");
    context_menu->addMenu(settings_menu);
    settings_menu->addAction("Show/hide grid lines");
    settings_menu->addAction("Show/hide axis legends");
    settings_menu->addSeparator();
    settings_menu->addAction("Preferences");

    context_menu->addAction("Switch to 2D view");

    //add external context menu
    if (context_add_)
    {
      context_menu->addSeparator();
      context_menu->addMenu(context_add_);
    }

    //evaluate menu
    if ((result = context_menu->exec(mapToGlobal(e->pos()))))
    {
      if (result->text() == "Preferences")
      {
        showCurrentLayerPreferences();
      }
      else if (result->text() == "Show/hide grid lines")
      {
        showGridLines(!gridLinesShown());
      }
      else if (result->text() == "Show/hide axis legends")
      {
        emit changeLegendVisibility();
      }
      else if (result->text() == "Layer" || result->text() == "Visible layer data")
      {
        saveCurrentLayer(result->text() == "Visible layer data");
      }
      else if (result->text() == "Layer meta data")
      {
        showMetaData(true);
      }
      else if (result->text() == "Switch to 2D view")
      {
        emit showCurrentPeaksAs2D();
      }
    }
    e->accept();
  }

  void Plot3DCanvas::saveCurrentLayer(bool visible)
  {
    const LayerDataBase& layer = getCurrentLayer();

    //determine proposed filename
    String proposed_name = param_.getValue("default_path").toString();
    if (visible == false && layer.filename != "")
    {
      proposed_name = layer.filename;
    }
    QString file_name = GUIHelpers::getSaveFilename(this, "Save file", proposed_name.toQString(), FileTypeList({FileTypes::MZML, FileTypes::MZDATA, FileTypes::MZXML}), true, FileTypes::MZML);
    if (file_name.isEmpty())
    {
      return;
    }

    if (visible)   //only visible data
    {
      ExperimentType out;
      getVisiblePeakData(out);
      addDataProcessing_(out, DataProcessing::FILTERING);
      FileHandler().storeExperiment(file_name, out, ProgressLogger::GUI);
    }
    else       //all data
    {
      FileHandler().storeExperiment(file_name, *layer.getPeakData(), ProgressLogger::GUI);
    }
  }

  void Plot3DCanvas::updateLayer(Size i)
  {
    selected_peak_.clear();
    recalculateRanges_(0, 1, 2);
    resetZoom(false); // no repaint as this is done in intensityModeChange_() anyway
    openglwidget()->recalculateDotGradient_(layers_.getLayer(i));
    intensityModeChange_();
    modificationStatus_(i, false);
  }

  void Plot3DCanvas::intensityModeChange_()
  {
    String gradient_str;
    if (intensity_mode_ == IM_LOG)
    {
      gradient_str = MultiGradient::getDefaultGradientLogarithmicIntensityMode().toString();
    }
    else // linear
    {
      gradient_str = linear_gradient_.toString();
    }
    for (Size i = 0; i < layers_.getLayerCount(); ++i)
    {
      layers_.getLayer(i).param.setValue("dot:gradient", gradient_str);
      openglwidget()->recalculateDotGradient_(layers_.getLayer(i));
    }
    PlotCanvas::intensityModeChange_();
  }

  void Plot3DCanvas::translateLeft_(Qt::KeyboardModifiers /*m*/)
  {
  }

  void Plot3DCanvas::translateRight_(Qt::KeyboardModifiers /*m*/)
  {
  }

  void Plot3DCanvas::translateForward_()
  {
  }

  void Plot3DCanvas::translateBackward_()
  {
  }

} //namespace
