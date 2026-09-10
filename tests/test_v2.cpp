#include "xword/xword.hpp"
#include "internal/Xml.hpp"
#include "check.hpp"
#include <filesystem>
#include <fstream>
#include <functional>
using namespace xword;
using namespace xword::internal;

static void saved(Document& doc,const std::string& path) {
    auto r=doc.saveDetailed(path); if(!r.success)std::cerr<<r.error.part<<": "<<r.error.message<<'\n'; assert(r.success);
}
static void rejects(const std::function<void()>& fn) { bool threw=false; try{fn();}catch(const std::exception&){threw=true;} assert(threw); }
int main() {
    {
        Document parity; parity.enableHeadingNumbering().enableTableNumbering("Table").setBodyFont("SimSun","Times New Roman");
        parity.addHeadingParagraph("Parity",1).setBookmark(TargetId("head",TargetKind::Heading));
        parity.addParagraph("Text").addRun("bold",RunStyle().bold()).addBreak(BreakType::Line);
        parity.addParagraph().addReference(TargetId("table",TargetKind::Table),ReferenceKind::Number);
        auto& t=parity.addTable(2,2); t.setHeaderRows(1).setBorderStyle(TableStyle::Grid).setCaption("Values").setBookmark(TargetId("table",TargetKind::Table)); t.cell(0,0).addParagraph("A"); t.cell(1,1).addParagraph("B");
        parity.addParagraph("Note").addFootnoteRef(parity.addFootnote("Footnote"));
        parity.addSection(SectionBreakType::Continuous).setPageNumbering(NumberFormat::LowerRoman,1);
        parity.currentSection().footer().addParagraph().addPageNumber(); parity.addParagraph("Last"); saved(parity,"v2_cpp_parity.docx");
    }
    Document doc;
    doc.setBodyFont(u8"宋体","Times New Roman").enableHeadingNumbering().enableImageNumbering("Figure").enableTableNumbering("Table").setEvenAndOddHeaders();
    auto& first=doc.currentSection(); first.setTitlePage().setPageNumbering(NumberFormat::LowerRoman,1);
    first.header(HeaderFooterType::First).addParagraph("Title header");
    first.header(HeaderFooterType::Even).addTable(1,1).cell(0,0).addParagraph("Even header table");
    first.footer().addParagraph().addPageNumber().addRun(" / ").addPageCount().addRun(" section ").addSectionPageCount();
    ParagraphStyle style; style.basedOn="Normal"; style.run.bold(); style.keepNext=Toggle::On; style.tabs.push_back({Length::cm(4),TabAlignment::Right,TabLeader::Dot}); style.lineSpacing=15; style.lineRule=LineRule::Exact; style.shading="EEEEEE"; style.border=Border();
    doc.registerParagraphStyle("Report",style).registerCharacterStyle("Emphasis",RunStyle().italic());
    TableStyleDefinition ts; ts.border=Border(); ts.shading="EFEFEF"; doc.registerTableStyle("ReportTable",ts);
    doc.addHeadingParagraph("Chapter",1).setBookmark(TargetId("chapter",TargetKind::Heading));
    doc.addParagraph().setStyleId("Report").addRun("off",RunStyle().bold(false)).addTab().addRun("after tab").setPageBreakBefore().setKeepTogether().setWidowControl(false);
    doc.addParagraph().addReference(TargetId("later",TargetKind::Table),ReferenceKind::Number).addReference(TargetId("later",TargetKind::Table),ReferenceKind::Page);
    auto& fn=doc.addFootnote(); fn.addParagraph("Footnote").addRun(" formatted",RunStyle().italic()); fn.addTable(1,1).cell(0,0).addParagraph("note table");
    auto& en=doc.addEndnote(); en.addParagraph("Endnote");
    doc.addParagraph("Notes").addFootnoteRef(fn.id()).addEndnoteRef(en.id()).addEquation("x^2");
    NoteOptions no; no.restart=NoteRestart::EachSection; first.setFootnoteOptions(no).setEndnoteOptions(no);
    int comment=doc.addComment("Review","Author","2026-09-10T00:00:00Z"); doc.addParagraph().startComment(comment).addRun("reviewed").endComment(comment);
    doc.addParagraph("link ").addHyperlink("OpenXML","https://learn.microsoft.com/office/open-xml/").addHyperlink("chapter","#chapter");
    auto& list=doc.addOrderedList(); list.setStart(3).addItem("three"); list.addItemParagraph("nested",1).addRun(" bold",RunStyle().bold());
    doc.addOrderedList().continueFrom(list).addItem("four");
    Table& table=doc.addTable(4,3); table.setStyleId("ReportTable").setHeaderRows(2).setRowAllowSplit(2,false).setRowHeight(0,Length::pt(25)).setWidth(Length::cm(15)).setColumnWidths({1,2,1}).setCaption("Results").setBookmark(TargetId("later",TargetKind::Table));
    table.mergeCells(2,0,3,1); table.cell(2,0).addParagraph("merged"); table.cell(0,0).setShading("AABBCC").setBorder(Border()).setMargins(Length::pt(2),Length::pt(2),Length::pt(3),Length::pt(3));
    rejects([&]{table.mergeCells(1,0,2,0);}); rejects([&]{table.cell(-1,0);}); rejects([&]{table.setColumnWidths({1,0,1});});
    // Table is the last block of the first section.
    auto& second=doc.addSection(SectionBreakType::Continuous); second.setPage(Page().setOrientation(Orientation::Landscape)).setPageNumbering(NumberFormat::Decimal,1).setTitlePage(false).setColumns(2,Length::cm(1)); second.clearHeader();
    assert(&first!=&second); first.header().addParagraph("stable section reference");
    doc.addParagraph("column one").addBreak(BreakType::Column).addRun("column two");
    doc.addSection(SectionBreakType::NextColumn).setColumnWidths({{Length::cm(8),Length::cm(1)},{Length::cm(9),Length()}});
    doc.addParagraph("next column"); doc.addSection(SectionBreakType::OddPage); doc.addSection(SectionBreakType::EvenPage); doc.addSection().setPage(Page()).setColumns(1);
    doc.addHeading("Deep heading",9); doc.addPageBreak(); doc.addTOC("1-9"); doc.addFigureTOC(); doc.addTableTOC();
    for(int i=0;i<6;++i) { BibliographySource s; s.tag="source"+std::to_string(i); s.type=static_cast<SourceType>(i); s.title="Source "+std::to_string(i); s.authors.push_back({"Ada","Lovelace","",""}); s.year="2026"; s.publisher="Publisher"; s.doi="10.1000/example"; doc.addSource(s); doc.addParagraph().addCitation(s.tag,"12"); }
    doc.addBibliography("References");
    DocumentProperties props; props.title="Report & <test>"; props.creator="Author"; doc.setProperties(props).setCustomProperty("Passed",{"true",PropertyType::Boolean}).setCustomProperty("Build",{"2",PropertyType::Number}).setCustomProperty("Date",{"2026-09-10T00:00:00Z",PropertyType::Date});
    auto imagePath=(std::filesystem::path(__FILE__).parent_path().parent_path()/"examples/res/image8.png").u8string();
    doc.addParagraph("before").addImage(imagePath).setSize(100,0).setAltText("inline image");
    doc.addImage(imagePath).setSize(200,0).setWrap(ImageWrap::Square).setPosition(Length::cm(1),Length::cm(1)).setCrop(.1,0,.1,0).setCaption("Diagram").setBookmark(TargetId("figure",TargetKind::Figure));
    doc.addParagraph().addReference(TargetId("figure",TargetKind::Figure),ReferenceKind::Number);
    auto& cell=doc.addTable(1,1).cell(0,0); cell.addParagraph("before image"); auto& img=cell.addImage(imagePath); img.setCaption("cell caption"); for(int i=0;i<10;++i)cell.addImage(imagePath).setSize(10,10); img.setAltText("stable image reference"); cell.addParagraph("after image");
    doc.currentSection().header().addImage(imagePath).setSize(80,0);
    saved(doc,"v2_features.docx"); saved(doc,"v2_repeat.docx");
    auto parts=readZip("v2_features.docx"),again=readZip("v2_repeat.docx"); assert(parts==again);
    pugi::xml_document xml; parseXml(xml,parts.at("word/document.xml"));
    assert(xml.select_nodes("//w:sectPr").size()==6); assert(xml.select_nodes("//w:tc//w:sectPr").empty()); assert(xml.select_nodes("/w:document/w:body/w:sectPr").size()==1);
    assert(xml.select_nodes("//w:sectPr/w:type[@w:val='nextColumn']").size()==1);
    assert(xml.select_nodes("//w:tblHeader").size()==2); assert(!xml.select_nodes("//w:cantSplit[@w:val='on']").empty());
    assert(xml.select_nodes("//w:r/m:oMath").empty()); assert(!xml.select_nodes("//w:p/m:oMath").empty());
    assert(parts.count("word/footnotes.xml")&&parts.count("word/endnotes.xml")&&parts.count("word/comments.xml")&&parts.count("customXml/item1.xml"));
    assert(parts.at("word/document.xml").find("PAGEREF later")!=std::string::npos); assert(parts.at("word/document.xml").find("REF later_n")!=std::string::npos);
    assert(xml.select_nodes("//w:bookmarkStart").size()==5);
    auto cellXml=xml.select_node("//w:tc[.//w:t[text()='before image']]").node(); std::string cellText=textContent(cellXml); assert(cellText.find("before image")<cellText.find("cell caption")&&cellText.find("cell caption")<cellText.find("after image"));

    Document invalid; invalid.addParagraph().addFootnoteRef(99); assert(invalid.saveDetailed("invalid.docx").error.code==SaveError::InvalidReference);
    Document invalidRef; invalidRef.addParagraph().addReference(TargetId("absent")); assert(!invalidRef.saveDetailed("invalid.docx").success);
    Document badRange; badRange.addParagraph().startBookmark(TargetId("unclosed")); assert(!badRange.saveDetailed("invalid.docx").success);
    Document missing; missing.addImage("missing-image.png"); auto warning=missing.saveDetailed("v2_missing.docx"); assert(warning.success&&warning.warnings.size()==1); SaveOptions strict; strict.missingImagesAreErrors=true; assert(missing.saveDetailed("v2_missing.docx",strict).error.code==SaveError::MissingResource);
    assert(!missing.saveDetailed("does-not-exist/output.docx").success);
    Document empty; saved(empty,"v2_empty.docx");

    // Split text across runs, preserve unrelated attributes and unknown parts.
    {
        ZipWriter zip("v2_template.docx");
        auto templateParts=readZip("v2_empty.docx");
        templateParts["word/document.xml"]=documentXml("w:document","<w:body><w:p><w:r><w:rPr><w:b/></w:rPr><w:t>${na</w:t></w:r><w:r><w:t>me} &amp; ${name}</w:t></w:r></w:p><w:p><w:r><w:t>${photo}</w:t></w:r></w:p><w:sectPr/></w:body>");
        templateParts["customXml/untouched.xml"]="<original value=\"${name}\"> bytes </original>";
        for(const auto& p:templateParts)assert(zip.addEntry(p.first,p.second)); assert(zip.finalize());
    }
    Document templ; assert(templ.open("v2_template.docx")); templ.set("name","A & <B>"); templ.setImage("photo",imagePath).setCaption("Inserted image"); saved(templ,"v2_template_out.docx");
    auto out=readZip("v2_template_out.docx"); parseXml(xml,out.at("word/document.xml")); assert(textContent(xml).find("A & <B> & A & <B>")!=std::string::npos); assert(out.at("word/document.xml").find("${photo}")==std::string::npos); assert(out.at("customXml/untouched.xml")=="<original value=\"${name}\"> bytes </original>");
    std::cout<<"XWord 2.0 feature and regression checks passed\n";
}
