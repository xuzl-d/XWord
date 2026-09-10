#include "xword/Table.hpp"
#include "internal/ZipWriter.hpp"
#include "internal/utf.hpp"
#include <numeric>
#include <array>
namespace xword {
using internal::xmlEscape;
namespace {
std::string borderXml(const Border& b,const char* tag,bool inside) {
    std::string x="<w:"+std::string(tag)+">";
    for(auto side:{"top","left","bottom","right","insideH","insideV"}) {
        if(!inside && std::string(side).find("inside")==0) continue;
        x+="<w:"+std::string(side)+" w:val=\""+xmlEscape(b.style)+"\" w:sz=\""+std::to_string(b.width.dxa()*8/20)+"\" w:color=\""+xmlEscape(b.color)+"\"/>";
    }
    return x+"</w:"+tag+">";
}
std::string marginsXml(const std::array<Length,4>& a,const char* tag) {
    std::string x="<w:"+std::string(tag)+">"; const char* sides[]={"top","bottom","left","right"};
    for(int i:{0,2,1,3}) x+="<w:"+std::string(sides[i])+" w:w=\""+std::to_string(a[i].dxa())+"\" w:type=\"dxa\"/>";
    return x+"</w:"+tag+">";
}
}
struct Cell::Impl { VAlignment align=VAlignment::Center; std::string shading,merge; int span=1; bool hidden=false; std::optional<Border> border; std::optional<std::array<Length,4>> margins; };
Cell::Cell():m_impl(std::make_unique<Impl>()) {}
Cell::~Cell()=default; Cell::Cell(Cell&&) noexcept=default; Cell& Cell::operator=(Cell&&) noexcept=default;
Cell& Cell::setVAlign(VAlignment v) { m_impl->align=v; return *this; }
VAlignment Cell::vAlign() const { return m_impl->align; }
Cell& Cell::setShading(const std::string& c) { m_impl->shading=c; return *this; }
Cell& Cell::setBorder(const Border& b) { m_impl->border=b; return *this; }
Cell& Cell::setMargins(Length t,Length b,Length l,Length r) { m_impl->margins=std::array<Length,4>{t,b,l,r}; return *this; }
void Cell::setVMerge(const std::string& v) { if(!v.empty()&&v!="restart"&&v!="continue") throw std::invalid_argument("Invalid vertical merge"); m_impl->merge=v; }
void Cell::setGridSpan(int n) { if(n<1) throw std::invalid_argument("Invalid grid span"); m_impl->span=n; }
void Cell::setHidden(bool on) { m_impl->hidden=on; }
const std::string& Cell::vMerge() const { return m_impl->merge; }
int Cell::gridSpan() const { return m_impl->span; }
bool Cell::hidden() const { return m_impl->hidden; }
std::string Cell::propertiesXml() const {
    std::string x;
    if(gridSpan()>1) x+="<w:gridSpan w:val=\""+std::to_string(gridSpan())+"\"/>";
    if(!vMerge().empty()) x+="<w:vMerge w:val=\""+vMerge()+"\"/>";
    if(m_impl->border) x+=borderXml(*m_impl->border,"tcBorders",false);
    if(!m_impl->shading.empty()) x+="<w:shd w:val=\"clear\" w:fill=\""+xmlEscape(m_impl->shading)+"\"/>";
    if(m_impl->margins) x+=marginsXml(*m_impl->margins,"tcMar");
    return x+"<w:vAlign w:val=\""+vAlignmentToString(vAlign())+"\"/>";
}
struct Table::Impl {
    int rows,cols,headers=0,width=0; bool autoFit=true;
    std::string styleId,caption,shading; TargetId target; RunStyle style;
    std::optional<Border> border;
    std::optional<std::array<Length,4>> margins;
    std::vector<std::vector<Cell>> cells;
    std::vector<double> ratios;
    std::vector<Toggle> split;
    std::vector<std::pair<int,bool>> heights;
    std::vector<std::vector<bool>> merged;
};
Table::Table(int r,int c):m_impl(std::make_unique<Impl>()) {
    if(r<=0||c<=0||c>63||r>100000) throw std::invalid_argument("Invalid table dimensions (columns 1..63)");
    m_impl->rows=r; m_impl->cols=c; m_impl->cells.resize(r); for(auto& row:m_impl->cells) row.resize(c);
    for(auto& row:m_impl->cells)for(auto& cell:row)cell.inheritStyle(&m_impl->style);
    m_impl->ratios.assign(c,1); m_impl->split.assign(r,Toggle::Inherit); m_impl->heights.assign(r,{0,false}); m_impl->merged.assign(r,std::vector<bool>(c,false));
}
Table::~Table()=default; Table::Table(Table&&) noexcept=default; Table& Table::operator=(Table&&) noexcept=default;
int Table::rows() const { return m_impl->rows; } int Table::cols() const { return m_impl->cols; }
Cell& Table::cell(int r,int c) { return m_impl->cells.at(r).at(c); }
const Cell& Table::cell(int r,int c) const { return m_impl->cells.at(r).at(c); }
Table& Table::setHeaderRow(int r) { return setHeaderRows(r+1); }
Table& Table::setHeaderRows(int n) { if(n<0||n>rows()) throw std::invalid_argument("Invalid header row count"); m_impl->headers=n; return *this; }
Table& Table::setRowAllowSplit(int r,bool on) { m_impl->split.at(r)=on?Toggle::On:Toggle::Off; return *this; }
Table& Table::setRowHeight(int r,Length h,bool exact) { if(h.dxa()<0) throw std::invalid_argument("Invalid row height"); m_impl->heights.at(r)={h.dxa(),exact}; return *this; }
Table& Table::setWidth(Length w) { if(w.dxa()<=0) throw std::invalid_argument("Invalid table width"); m_impl->width=w.dxa(); return *this; }
Table& Table::setAutoFit(bool on) { m_impl->autoFit=on; return *this; }
Table& Table::setStyleId(const std::string& id) { m_impl->styleId=id; return *this; }
Table& Table::setBorderStyle(TableStyle s) { Border b; b.style=s==TableStyle::None?"nil":"single"; if(s==TableStyle::Light) b.color="D9D9D9"; m_impl->border=b; return *this; }
Table& Table::setBorder(const Border& b) { m_impl->border=b; return *this; }
Table& Table::setShading(const std::string& s) { m_impl->shading=s; return *this; }
Table& Table::setCellMargins(Length t,Length b,Length l,Length r) { m_impl->margins=std::array<Length,4>{t,b,l,r}; return *this; }
Table& Table::setStyle(const RunStyle& s) { m_impl->style=s; return *this; }
RunStyle& Table::getStyle() { return m_impl->style; }
Table& Table::setVAlign(VAlignment v) { for(auto& row:m_impl->cells) for(auto& c:row)c.setVAlign(v); return *this; }
Table& Table::setCaption(const std::string& s) { m_impl->caption=s; return *this; }
Table& Table::setCaption(const std::wstring& s) { return setCaption(internal::wstring_to_utf8(s)); }
Table& Table::setBookmark(const TargetId& t) { m_impl->target=t; return *this; }
const std::string& Table::caption() const { return m_impl->caption; }
Table& Table::setColumnWidth(int c,double ratio) { if(!std::isfinite(ratio)||ratio<=0) throw std::invalid_argument("Column ratio must be positive"); m_impl->ratios.at(c)=ratio; m_impl->autoFit=false; return *this; }
Table& Table::setColumnWidths(const std::vector<double>& ratios) { if(ratios.size()!=static_cast<size_t>(cols())) throw std::invalid_argument("Column ratio count mismatch"); for(auto n:ratios) if(!std::isfinite(n)||n<=0) throw std::invalid_argument("Column ratio must be positive"); m_impl->ratios=ratios; m_impl->autoFit=false; return *this; }
double Table::getColumnWidth(int c) const { return m_impl->ratios.at(c); }
Table& Table::mergeCells(int r1,int c1,int r2,int c2) {
    cell(r1,c1); cell(r2,c2); if(r1>r2||c1>c2) throw std::invalid_argument("Reversed merge range");
    if(r1==r2&&c1==c2) return *this;
    for(int r=r1;r<=r2;++r) for(int c=c1;c<=c2;++c) if(m_impl->merged[r][c]) throw std::invalid_argument("Overlapping merge range");
    for(int r=r1;r<=r2;++r) for(int c=c1;c<=c2;++c) {
        m_impl->merged[r][c]=true; auto& v=cell(r,c);
        if(c>c1) v.setHidden(true);
        else { v.setGridSpan(c2-c1+1); if(r2>r1) v.setVMerge(r==r1?"restart":"continue"); }
    }
    return *this;
}
std::string Table::toXml() const {
    std::string x="<w:tbl xw:width=\""+std::to_string(m_impl->width)+"\" xw:caption=\""+xmlEscape(caption())+"\" xw:target=\""+xmlEscape(m_impl->target.name)+"\" xw:kind=\""+std::to_string(static_cast<int>(m_impl->target.kind))+"\"><w:tblPr>";
    if(!m_impl->styleId.empty()) x+="<w:tblStyle w:val=\""+xmlEscape(m_impl->styleId)+"\"/>";
    x+="<w:tblW w:w=\""+std::to_string(m_impl->width?m_impl->width:5000)+"\" w:type=\""+(m_impl->width?"dxa":"pct")+"\"/><w:jc w:val=\"center\"/>";
    if(m_impl->border) x+=borderXml(*m_impl->border,"tblBorders",true);
    if(!m_impl->shading.empty()) x+="<w:shd w:val=\"clear\" w:fill=\""+xmlEscape(m_impl->shading)+"\"/>";
    x+="<w:tblLayout w:type=\""+std::string(m_impl->autoFit?"autofit":"fixed")+"\"/>";
    if(m_impl->margins) x+=marginsXml(*m_impl->margins,"tblCellMar");
    x+="</w:tblPr><w:tblGrid>";
    double total=std::accumulate(m_impl->ratios.begin(),m_impl->ratios.end(),0.0);
    for(auto ratio:m_impl->ratios) x+="<w:gridCol xw:ratio=\""+std::to_string(ratio/total)+"\"/>";
    x+="</w:tblGrid>";
    for(int r=0;r<rows();++r) {
        x+="<w:tr><w:trPr>";
        if(m_impl->split[r]!=Toggle::Inherit) x+="<w:cantSplit w:val=\""+std::string(m_impl->split[r]==Toggle::Off?"on":"off")+"\"/>";
        if(m_impl->heights[r].first>0) x+="<w:trHeight w:val=\""+std::to_string(m_impl->heights[r].first)+"\" w:hRule=\""+(m_impl->heights[r].second?"exact":"atLeast")+"\"/>";
        if(r<m_impl->headers) x+="<w:tblHeader/>";
        x+="</w:trPr>";
        for(int c=0;c<cols();++c) {
            const auto& v=cell(r,c); if(v.hidden()) continue;
            if(c+v.gridSpan()>cols()) throw std::invalid_argument("Cell span exceeds table grid");
            double ratio=0; for(int j=c;j<c+v.gridSpan();++j) ratio+=m_impl->ratios[j]/total;
            x+="<w:tc xw:ratio=\""+std::to_string(ratio)+"\"><w:tcPr>"+v.propertiesXml()+"</w:tcPr>"+v.Content::toXml();
            if(v.empty())x+="<w:p/>";
            x+="</w:tc>";
        }
        x+="</w:tr>";
    }
    return x+"</w:tbl>";
}
}
