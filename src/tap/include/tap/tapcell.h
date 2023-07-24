/////////////////////////////////////////////////////////////////////////////
//
// BSD 3-Clause License
//
// Copyright (c) 2019, The Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////////

#include "odb/db.h"

namespace ord {
class OpenRoad;
}

namespace utl {
class Logger;
}

namespace odb {
class dbDatabase;
class dbTech;
class dbBlock;
}  // namespace odb

namespace tap {

enum LocationType
{
  AboveMacro,
  BelowMacro,
  NoMacro,
};

struct Options
{
  odb::dbMaster* endcap_master = nullptr;
  odb::dbMaster* tapcell_master = nullptr;
  int dist = -1;    // default = 2um
  int halo_x = -1;  // default = 2um
  int halo_y = -1;  // default = 2um
  odb::dbMaster* cnrcap_nwin_master = nullptr;
  odb::dbMaster* cnrcap_nwout_master = nullptr;
  odb::dbMaster* tap_nwintie_master = nullptr;
  odb::dbMaster* tap_nwin2_master = nullptr;
  odb::dbMaster* tap_nwin3_master = nullptr;
  odb::dbMaster* tap_nwouttie_master = nullptr;
  odb::dbMaster* tap_nwout2_master = nullptr;
  odb::dbMaster* tap_nwout3_master = nullptr;
  odb::dbMaster* incnrcap_nwin_master = nullptr;
  odb::dbMaster* incnrcap_nwout_master = nullptr;

  bool addBoundaryCells() const
  {
    return tap_nwintie_master && tap_nwin2_master && tap_nwin3_master
           && tap_nwouttie_master && tap_nwout2_master && tap_nwout3_master
           && incnrcap_nwin_master && incnrcap_nwout_master;
  }
};

class Tapcell
{
 public:
  Tapcell();
  void init(odb::dbDatabase* db, utl::Logger* logger);
  void setTapPrefix(const std::string& tap_prefix);
  void setEndcapPrefix(const std::string& endcap_prefix);
  void clear();
  void run(const Options& options);
  void cutRows(const Options& options);
  void reset();
  int removeCells(const std::string& prefix);

 private:
  struct FilledSites
  {
    int yMin = 0;
    int xMin = 0;
    int xMax = 0;
  };
  // Cells placed at corners of macros & corners of core area
  struct CornercapMasters
  {
    odb::dbMaster* nwin_master;
    odb::dbMaster* nwout_master;
  };
  using RowFills = std::map<int, std::vector<std::vector<int>>>;

  std::vector<odb::dbBox*> findBlockages();
  const std::pair<int, int> getMinMaxX(
      const std::vector<std::vector<odb::dbRow*>>& rows);
  RowFills findRowFills();
  odb::dbMaster* pickCornerMaster(const LocationType top_bottom,
                                  const odb::dbOrientType& ori,
                                  odb::dbMaster* cnrcap_nwin_master,
                                  odb::dbMaster* cnrcap_nwout_master,
                                  odb::dbMaster* endcap_master);
  bool checkSymmetry(odb::dbMaster* master, const odb::dbOrientType& ori);
  LocationType getLocationType(int x,
                               const std::vector<odb::dbRow*>& rows_above,
                               const std::vector<odb::dbRow*>& rows_below);
  void makeInstance(odb::dbBlock* block,
                    odb::dbMaster* master,
                    const odb::dbOrientType& orientation,
                    int x,
                    int y,
                    const std::string& prefix);
  bool isXInRow(int x, const std::vector<odb::dbRow*>& subrow);
  bool checkIfFilled(int x,
                     int width,
                     const odb::dbOrientType& orient,
                     const std::vector<std::vector<int>>& row_insts);
  int insertAtTopBottom(const std::vector<std::vector<odb::dbRow*>>& rows,
                        const std::vector<odb::dbMaster*>& masters,
                        odb::dbMaster* endcap_master,
                        const std::string& prefix);
  void insertAtTopBottomHelper(
      odb::dbBlock* block,
      int top_bottom,
      bool is_macro,
      const odb::dbOrientType& ori,
      int x_start,
      int x_end,
      int lly,
      odb::dbMaster* tap_nwintie_master,
      odb::dbMaster* tap_nwin2_master,
      odb::dbMaster* tap_nwin3_master,
      odb::dbMaster* tap_nwouttie_master,
      odb::dbMaster* tap_nwout2_master,
      odb::dbMaster* tap_nwout3_master,
      const std::vector<std::vector<int>>& row_fill_check,
      const std::string& prefix);
  int insertAroundMacros(const std::vector<std::vector<odb::dbRow*>>& rows,
                         const std::vector<odb::dbMaster*>& masters,
                         odb::dbMaster* corner_master,
                         const std::string& prefix);
  std::map<std::pair<int, int>, std::vector<int>> getMacroOutlines(
      const std::vector<std::vector<odb::dbRow*>>& rows);
  int insertEndcaps(const std::vector<std::vector<odb::dbRow*>>& rows,
                    odb::dbMaster* endcap_master,
                    const CornercapMasters& masters);
  std::vector<std::vector<odb::dbRow*>> organizeRows();
  int insertTapcells(const std::vector<std::vector<odb::dbRow*>>& rows,
                     odb::dbMaster* tapcell_master,
                     int dist);

  int defaultDistance() const;

  odb::dbDatabase* db_ = nullptr;
  utl::Logger* logger_ = nullptr;
  int phy_idx_ = 0;
  std::vector<FilledSites> filled_sites_;
  std::string tap_prefix_;
  std::string endcap_prefix_;
};

}  // namespace tap
