#include "xword/xword.hpp"
#include "internal/Xml.hpp"
#include "check.hpp"
#include <filesystem>
#include <fstream>
using namespace xword;
using namespace xword::internal;
static void save(Document& d,const std::string& path) { auto r=d.saveDetailed(path); if(!r.success)std::cerr<<r.error.message<<'\n'; assert(r.success); }
int main() {
    namespace fs=std::filesystem;
    auto image=(fs::path(__FILE__).parent_path().parent_path()/"examples/res/image8.png").u8string();
    Document formatting; formatting.setBodyFont("SimSun","Arial","Calibri"); auto& formattedTable=formatting.addTable(1,1); formattedTable.setStyle(RunStyle().fontSize(12).bold()); formattedTable.getStyle().fontSize(18); formattedTable.cell(0,0).getStyle().bold(false); formattedTable.cell(0,0).addParagraph("cell defaults"); save(formatting,"v2_style_defaults.docx"); auto formatted=readZip("v2_style_defaults.docx"); pugi::xml_document formatXml; parseXml(formatXml,formatted.at("word/styles.xml")); assert(std::string(formatXml.select_node("//w:docDefaults//w:rFonts").node().attribute("w:hAnsi").value())=="Calibri"); parseXml(formatXml,formatted.at("word/document.xml")); assert(formatXml.select_nodes("//w:rPr/w:sz[@w:val='36']").size()==1); assert(formatXml.select_nodes("//w:rPr/w:b[@w:val='0']").size()==1);
    Document original; ParagraphStyle templateBase; templateBase.basedOn="Normal"; original.registerParagraphStyle("TemplateBase",templateBase); original.addParagraph("Original"); save(original,"v2_atomic.docx"); auto before=readZip("v2_atomic.docx");
    Document bad; bad.addParagraph().addReference(TargetId("missing")); assert(!bad.saveDetailed("v2_atomic.docx").success); assert(readZip("v2_atomic.docx")==before);
    Document utf; utf.addParagraph(std::string("bad\x01",4)); assert(!utf.saveDetailed("bad.xml.docx").success);
    Document invalidUtf; invalidUtf.addParagraph(std::string("\xC0\xAF",2)); assert(!invalidUtf.saveDetailed("bad-utf.docx").success);
    Document order; TargetId reversed("reversed"); order.addParagraph().endBookmark(reversed).startBookmark(reversed); assert(!order.saveDetailed("bad-order.docx").success);
    Document noteContext; noteContext.currentSection().header().addParagraph().addFootnoteRef(noteContext.addFootnote("note")); assert(!noteContext.saveDetailed("bad-note.docx").success);
    Document inheritance; auto& first=inheritance.currentSection(); first.header().addParagraph("first");
    inheritance.addParagraph("section one"); inheritance.addSection(); inheritance.addParagraph("inherit"); inheritance.addSection().clearHeader(); inheritance.addParagraph("blank");
    save(inheritance,"v2_headers.docx"); auto h=readZip("v2_headers.docx"); pugi::xml_document xml; parseXml(xml,h.at("word/document.xml"));
    auto sections=xml.select_nodes("//w:sectPr"); assert(sections.size()==3); assert(sections[0].node().child("w:headerReference")); assert(!sections[1].node().child("w:headerReference")); assert(sections[2].node().child("w:headerReference")); assert(h.count("word/header3_default.xml"));
    auto src=before; src["word/document.xml"]="<q:document xmlns:q=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><q:body><q:p><q:r><q:t>$</q:t></q:r><q:r><q:t>{name}</q:t></q:r></q:p><q:p><q:r><q:t>{%if enabled%}</q:t></q:r></q:p><q:p><q:r><q:t>yes</q:t></q:r></q:p><q:p><q:r><q:t>{%else%}</q:t></q:r></q:p><q:p><q:r><q:t>no</q:t></q:r></q:p><q:p><q:r><q:t>{%endif%}</q:t></q:r></q:p><q:sectPr/></q:body></q:document>";
    src["word/header1.xml"]=documentXml("w:hdr","<w:p><w:r><w:t>$</w:t></w:r><w:r><w:t>{name}</w:t></w:r></w:p>");
    src["customXml/preserve.xml"]="<?xml version='1.0'?>\n<private attr='${name}'>  keep \n</private>";
    { ZipWriter zip("v2_split_template.docx"); for(const auto& e:src)assert(zip.addEntry(e.first,e.second)); assert(zip.finalize()); }
    Document templ; assert(templ.open("v2_split_template.docx")); templ.set("name",u8"中文 & <value>").set("enabled",true); ParagraphStyle derived; derived.basedOn="TemplateBase"; derived.run.italic(); templ.registerParagraphStyle("TemplateDerived",derived); save(templ,"v2_split_result.docx");
    auto t=readZip("v2_split_result.docx"); parseXml(xml,t.at("word/document.xml")); auto text=textContent(xml); assert(text.find(u8"中文 & <value>")!=std::string::npos); assert(text.find("yes")!=std::string::npos&&text.find("no")==std::string::npos); assert(t.at("customXml/preserve.xml")==src.at("customXml/preserve.xml")); parseXml(xml,t.at("word/header1.xml")); assert(textContent(xml)==u8"中文 & <value>");
    bool denied=false; try{templ.addParagraph("ignored");}catch(const std::invalid_argument&){denied=true;} assert(denied);
    // A source image with one requested dimension retains its intrinsic ratio.
    auto size=computeImageSize(image,100,0,0),intrinsic=computeImageSize(image,0,0,0); assert(size.widthEmu==952500); assert(std::abs(static_cast<double>(size.widthEmu)/size.heightEmu-static_cast<double>(intrinsic.widthEmu)/intrinsic.heightEmu)<.001);
    auto unicodePath=fs::u8path(u8"中文路径/生成.docx"); fs::create_directories(unicodePath.parent_path()); save(original,unicodePath.u8string()); assert(original.save(unicodePath.wstring())); assert(original.saveDetailed(unicodePath.wstring()).success); Document unicodeTemplate; assert(unicodeTemplate.open(unicodePath.u8string())); assert(unicodeTemplate.open(unicodePath.wstring())); auto unicodeWide=fs::u8path(u8"中文路径/宽路径生成.docx"); assert(original.save(unicodeWide.c_str())); assert(unicodeTemplate.open(unicodeWide.wstring()));
    const wchar_t* nullWide=nullptr; assert(!unicodeTemplate.open(nullWide)); assert(!original.save(nullWide)); auto nullSave=original.saveDetailed(nullWide); assert(!nullSave.success); assert(nullSave.error.code==SaveError::InvalidArgument);
    bool badFormat=false; try { numberFormatName(static_cast<NumberFormat>(99)); } catch(const std::invalid_argument&) { badFormat=true; } assert(badFormat);
    { std::ofstream svg("v2_image.svg"); svg<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"200\" height=\"100\" viewBox=\"0 0 200 100\"><rect width=\"200\" height=\"100\" fill=\"blue\"/></svg>"; }
    fs::create_directories("media-a");fs::create_directories("media-b");
    fs::copy_file(fs::u8path(image),"media-a/shared.png",fs::copy_options::overwrite_existing);
    fs::copy_file(fs::u8path(image),"media-b/shared.png",fs::copy_options::overwrite_existing);
    Document pictures; pictures.enableHeadingNumbering().enableImageNumbering("Figure",CaptionNumStyle::ByChapter);
    pictures.addHeading("First",1); pictures.addImage("media-a/shared.png").setCaption("first image"); pictures.addHeading("Second",1); pictures.addImage("media-b/shared.png").setCaption("second image");
    for(int i=0;i<5;++i)pictures.addImage(image).setWrap(static_cast<ImageWrap>(i)).setSize(80,40).setAltText("alternative","title").setPosition(Length::pt(10),Length::pt(15),PositionRelative::Page,PositionRelative::Page);
    pictures.addImage("v2_image.svg").setSvgFallback(image).setSize(150,0).setCrop(.1,.1,.1,.1);
    pictures.addSection().setColumns(2);
    pictures.addImage(image).setSize(5000,1000);
    pictures.addTable(1,1).cell(0,0).addTable(1,1).cell(0,0).addParagraph("nested table at end");
    save(pictures,"v2_images.docx"); auto images=readZip("v2_images.docx"); parseXml(xml,images.at("word/document.xml"));
    assert(xml.select_nodes("//wp:anchor").size()==4); assert(xml.select_nodes("//a:extLst/a:ext/asvg:svgBlip").size()==1);
    auto drawings=xml.select_nodes("//wp:extent"); assert(drawings[drawings.size()-1].node().attribute("cx").as_int()<=pictures.currentSection().columnWidth()*635);
    int media=0;for(const auto& e:images)if(e.first.find("word/media/")==0)++media; assert(media==4);
    auto nested=xml.select_node("//w:tc[w:tbl]").node(); assert(std::string(nested.last_child().name())=="w:p");
    std::cout<<"Template, image, ownership and failure regressions passed\n";
}
