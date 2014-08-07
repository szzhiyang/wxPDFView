#include "private/PDFViewBookmarks.h"
#include "private/PDFViewImpl.h"

class wxPDFViewBookmarkImpl: public wxPDFViewBookmark
{
public:
	wxPDFViewBookmarkImpl(CPDF_BookmarkTree& bmTree, CPDF_Bookmark& bookmark):
		m_bookmark(bookmark)
	{
		CFX_ByteString bookmarkTitleSDK = bookmark.GetTitle().UTF8Encode();
		m_title = wxString::FromUTF8(bookmarkTitleSDK, bookmarkTitleSDK.GetLength());
		CPDF_Bookmark child = bmTree.GetFirstChild(bookmark);
		while (child)
		{
			wxSharedPtr<wxPDFViewBookmark> newBM(new wxPDFViewBookmarkImpl(bmTree, child));
			push_back(newBM);
			child = bmTree.GetNextSibling(child);
		}
	}

	virtual wxString GetTitle() const
	{
		return m_title;
	}

	virtual void Navigate(wxPDFView* pdfView)
	{
		CPDF_Document* doc = (CPDF_Document*) pdfView->GetImpl()->GetDocument();
		CPDF_Dest dest = m_bookmark.GetDest(doc);
		if (!dest)
		{
			CPDF_Action action = m_bookmark.GetAction();
			dest = action.GetDest(doc);
		}

		if (dest)
			pdfView->SetCurrentPage(dest.GetPageIndex(doc));
	}

private:
	wxString m_title;
	CPDF_Bookmark m_bookmark;
};

wxPDFViewBookmarks::wxPDFViewBookmarks(FPDF_DOCUMENT doc):
	m_tree((CPDF_Document*)doc)
{
	CPDF_Bookmark emptyBM;
	CPDF_Bookmark rootBM = m_tree.GetFirstChild(emptyBM);
	if (rootBM)
		m_root.reset(new wxPDFViewBookmarkImpl(m_tree, emptyBM));
}
