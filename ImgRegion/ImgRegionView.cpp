#include "stdafx.h"
#include "ImgRegionView.h"
#include "MainApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CImgRegionView, CImageView)

BEGIN_MESSAGE_MAP(CImgRegionView, CImageView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_ZOOM_IN, &CImgRegionView::OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, &CImgRegionView::OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, &CImgRegionView::OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, &CImgRegionView::OnUpdateZoomOut)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CImgRegionView::CImgRegionView(void)
	: m_nCurrCursor(CursorTypeArrow)
	, m_nCurrImageSize(10)
	, m_DragState(DragStateNone)
	, m_nSelectedHandle(HandleTypeNone)
{
}

CImgRegionDoc * CImgRegionView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CImgRegionDoc)));
	return (CImgRegionDoc *)m_pDocument;
}

static const Gdiplus::Color HANDLE_COLOR(255,0,0,255);

void CImgRegionView::OnDraw(CDC * pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	HTREEITEM hItemRoot = pDoc->m_TreeCtrl.GetRootItem();
	ASSERT(hItemRoot);
	if (!hItemRoot)
		return;

	CImgRegion * pRegRoot = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItemRoot);
	ASSERT(pRegRoot);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(pDC);
	CBitmap * oldBmp = dcMemory.SelectObject(&bmp);
	dcMemory.FillSolidRect(&rectClient, RGB(192,192,192));

	Gdiplus::Graphics grap(dcMemory.GetSafeHdc());
	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(255,192,192,192));
	grap.FillRectangle(&bkBrush, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height());
	grap.TranslateTransform(-(float)GetScrollPos(SB_HORZ), -(float)GetScrollPos(SB_VERT));
	grap.ScaleTransform(
		(float)m_ImageSizeTable[m_nCurrImageSize].cx / pRegRoot->m_rc.Width(),
		(float)m_ImageSizeTable[m_nCurrImageSize].cy / pRegRoot->m_rc.Height());

	DrawRegionNode(grap, &pDoc->m_TreeCtrl, hItemRoot);

	grap.ResetTransform();

	HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
	CPoint ptTopLeft;
	if(hSelected && pDoc->LocalToRoot(hSelected, CPoint(0,0), ptTopLeft))
	{
		CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
		ASSERT(pReg);

		CRect rect(ptTopLeft, pReg->m_rc.Size());
		CWindowDC dc(this);
		PrepareDC(&dc, pRegRoot->m_rc,
			CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));
		dc.LPtoDP(&rect.TopLeft());
		dc.LPtoDP(&rect.BottomRight());

		DrawRectHandle(grap, rect);

		CPoint ptCenter = rect.CenterPoint();
		DrawSmallHandle(grap, CPoint(rect.left, rect.top), m_nSelectedHandle == HandleTypeLeftTop);
		DrawSmallHandle(grap, CPoint(ptCenter.x, rect.top), m_nSelectedHandle == HandleTypeCenterTop);
		DrawSmallHandle(grap, CPoint(rect.right, rect.top), m_nSelectedHandle == HandleTypeRightTop);
		DrawSmallHandle(grap, CPoint(rect.left, ptCenter.y), m_nSelectedHandle == HandleTypeLeftMiddle);
		DrawSmallHandle(grap, CPoint(rect.right, ptCenter.y), m_nSelectedHandle == HandleTypeRightMiddle);
		DrawSmallHandle(grap, CPoint(rect.left, rect.bottom), m_nSelectedHandle == HandleTypeLeftBottom);
		DrawSmallHandle(grap, CPoint(ptCenter.x, rect.bottom), m_nSelectedHandle == HandleTypeCenterBottom);
		DrawSmallHandle(grap, CPoint(rect.right, rect.bottom), m_nSelectedHandle == HandleTypeRightBottom);
	}

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dcMemory, 0, 0, SRCCOPY);
}

void CImgRegionView::DrawRectHandle(Gdiplus::Graphics & grap, const CRect & rectHandle)
{
	Gdiplus::Pen pen(HANDLE_COLOR, 1.0f);
	pen.SetDashStyle(Gdiplus::DashStyleDash);
	float dashValue[] = { 10.0f, 4.0f };
	pen.SetDashPattern(dashValue, _countof(dashValue));
	grap.DrawRectangle(&pen, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
}

static const int HANDLE_WIDTH = 4;

void CImgRegionView::DrawSmallHandle(Gdiplus::Graphics & grap, const CPoint & ptHandle, BOOL bSelected)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	Gdiplus::Pen pen(HANDLE_COLOR,1.0f);
	Gdiplus::SolidBrush brush(bSelected ? HANDLE_COLOR : Gdiplus::Color(255,255,255,255));
	grap.FillRectangle(&brush, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
	grap.DrawRectangle(&pen, rectHandle.left, rectHandle.top, rectHandle.Width(), rectHandle.Height());
}

BOOL CImgRegionView::CheckSmallHandle(const CPoint & ptHandle, const CPoint & ptMouse)
{
	CRect rectHandle(ptHandle.x - HANDLE_WIDTH, ptHandle.y - HANDLE_WIDTH, ptHandle.x + HANDLE_WIDTH, ptHandle.y + HANDLE_WIDTH);
	return rectHandle.PtInRect(ptMouse);
}

void CImgRegionView::DrawRegionNode(Gdiplus::Graphics & grap, CTreeCtrl * pTreeCtrl, HTREEITEM hItem, const CPoint & ptOff)
{
	CImgRegion * pReg = (CImgRegion *)pTreeCtrl->GetItemData(hItem);
	ASSERT(pReg);

	CRect rectNode(pReg->m_rc);
	rectNode.OffsetRect(ptOff);

	if(pReg->m_image)
	{
		DrawRegionImage(grap, pReg->m_image.get(), rectNode, pReg->m_border, pReg->m_color.GetAlpha());
	}
	else
	{
		Gdiplus::SolidBrush brush(pReg->m_color);
		grap.FillRectangle(&brush, rectNode.left, rectNode.top, rectNode.Width(), rectNode.Height());
	}

	if(pReg->m_font)
	{
		CString strInfo;
		strInfo.Format(_T("x:%d y:%d w:%d h:%d"), pReg->m_rc.left, pReg->m_rc.top, pReg->m_rc.Width(), pReg->m_rc.Height());

		Gdiplus::RectF rectF((float)rectNode.left, (float)rectNode.top, (float)rectNode.Width(), (float)rectNode.Height());
		Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 0, 0, 255));
		Gdiplus::StringFormat strFormat(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoClip);
		strFormat.SetTrimming(Gdiplus::StringTrimmingNone);
		grap.DrawString(strInfo, strInfo.GetLength(), pReg->m_font.get(), rectF, &strFormat, &solidBrush);
	}

	HTREEITEM hChild = pTreeCtrl->GetChildItem(hItem);
	for(; NULL != hChild; hChild = pTreeCtrl->GetNextSiblingItem(hItem))
	{
		DrawRegionNode(grap, pTreeCtrl, hChild, CPoint(ptOff.x + pReg->m_rc.left, ptOff.y + pReg->m_rc.top));
	}
}

void CImgRegionView::DrawRegionImage(Gdiplus::Graphics & grap, Gdiplus::Image * img, const CRect & dstRect, const Vector4i & border, int alpha)
{
		Gdiplus::ColorMatrix colorMatrix = {	1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 0.0f, alpha / 255.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
		//imageAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
		Gdiplus::InterpolationMode oldInterpolationMode = grap.GetInterpolationMode();
		grap.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		Gdiplus::PixelOffsetMode oldPixelOffsetMode = grap.GetPixelOffsetMode();
		grap.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.top, border.x, border.y),
			0, 0, border.x, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.top, dstRect.Width() - border.x - border.z, border.y),
			border.x, 0, img->GetWidth() - border.x - border.z, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.top, border.x, border.y),
			img->GetWidth() - border.z, 0, border.z, border.y, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.top + border.y, border.x, dstRect.Height() - border.y - border.w),
			0, border.y, border.x, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.top + border.y, dstRect.Width() - border.x - border.z, dstRect.Height() - border.y - border.w),
			border.x, border.y, img->GetWidth() - border.x - border.z, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.top + border.y, border.x, dstRect.Height() - border.y - border.w),
			img->GetWidth() - border.z, border.y, border.z, img->GetHeight() - border.y - border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left, dstRect.bottom - border.w, border.x, border.w),
			0, img->GetHeight() - border.w, border.x, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.left + border.x, dstRect.bottom - border.w, dstRect.Width() - border.x - border.z, border.w),
			border.x, img->GetHeight() - border.w, img->GetWidth() - border.x - border.z, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.DrawImage(img, Gdiplus::Rect(dstRect.right - border.z, dstRect.bottom - border.w, border.x, border.w),
			img->GetWidth() - border.z, img->GetHeight() - border.w, border.z, border.w, Gdiplus::UnitPixel, &imageAtt);

		grap.SetInterpolationMode(oldInterpolationMode);
		grap.SetPixelOffsetMode(oldPixelOffsetMode);
}

BOOL CImgRegionView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

int CImgRegionView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CImageView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_hCursor[CursorTypeArrow] = theApp.LoadCursor(IDC_CURSOR1);
	m_hCursor[CursorTypeCross] = theApp.LoadCursor(IDC_CURSOR2);

	return 0;
}

void CImgRegionView::OnInitialUpdate()
{
	CImageView::OnInitialUpdate();

	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	HTREEITEM hItem = pDoc->m_TreeCtrl.GetRootItem();
	ASSERT(hItem);
	if (!hItem)
		return;

	CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItem);
	ASSERT(pReg);

	UpdateImageSizeTable(pReg->m_rc.Size());

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize]);
}

void CImgRegionView::UpdateImageSizeTable(const CSize & sizeRoot)
{
	for(int i = 0; i < _countof(m_ImageSizeTable); i++)
	{
		m_ImageSizeTable[i] = CSize((int)(sizeRoot.cx * ZoomTable[i]), (int)(sizeRoot.cy * ZoomTable[i]));
	}
}

void CImgRegionView::OnSize(UINT nType, int cx, int cy)
{
	CImageView::OnSize(nType, cx, cy);

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize], TRUE, CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT)));
}

void CImgRegionView::OnZoomIn()
{
	CRect rectClient;
	GetClientRect(rectClient);

	ZoomImage(m_nCurrImageSize - 1, rectClient.CenterPoint());
}

void CImgRegionView::OnUpdateZoomIn(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nCurrImageSize > 0);
}

void CImgRegionView::OnZoomOut()
{
	CRect rectClient;
	GetClientRect(rectClient);

	ZoomImage(m_nCurrImageSize + 1, rectClient.CenterPoint());
}

void CImgRegionView::OnUpdateZoomOut(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nCurrImageSize < _countof(m_ImageSizeTable) - 1);
}

void CImgRegionView::ZoomImage(int ImageSizeIdx, const CPoint & ptLook, BOOL bRedraw)
{
	CImgRegionDoc * pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect SrcImageRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]);

	m_nCurrImageSize = max(0, min((int)_countof(m_ImageSizeTable) - 1, ImageSizeIdx));

	my::Vector2 center = MapPoint(my::Vector2((float)ptLook.x, (float)ptLook.y), SrcImageRect, CRect(CPoint(0, 0), m_ImageSizeTable[m_nCurrImageSize]));

	SetScrollSizes(m_ImageSizeTable[m_nCurrImageSize], bRedraw, CPoint((int)(center.x - ptLook.x), (int)(center.y - ptLook.y)));

	if(bRedraw)
		Invalidate(TRUE);
}

void CImgRegionView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(!((1 << 14) & nFlags) && DragStateNone == m_DragState)
		{
			m_nCurrCursor = CursorTypeCross;
			SetCursor(m_hCursor[m_nCurrCursor]);
		}
		break;

	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		if(DragStateNone == m_DragState)
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			CSize dragOff(
				nChar == VK_LEFT ? -1 : (nChar == VK_RIGHT ? 1 : 0),
				nChar == VK_UP ? -1 : (nChar == VK_DOWN ? 1 : 0));


			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			ASSERT(hSelected);
			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				switch(m_nSelectedHandle)
				{
				case HandleTypeLeftTop:
					m_DragHandlePos.x = (pReg->m_rc.left += dragOff.cx);
					m_DragHandlePos.y = (pReg->m_rc.top += dragOff.cy);
					break;
				case HandleTypeCenterTop:
					m_DragHandlePos.y = (pReg->m_rc.top += dragOff.cy);
					break;
				case HandleTypeRightTop:
					m_DragHandlePos.x = (pReg->m_rc.right += dragOff.cx);
					m_DragHandlePos.y = (pReg->m_rc.top += dragOff.cy);
					break;
				case HandleTypeLeftMiddle:
					m_DragHandlePos.x = (pReg->m_rc.left += dragOff.cx);
					break;
				case HandleTypeRightMiddle:
					m_DragHandlePos.x = (pReg->m_rc.right += dragOff.cx);
					break;
				case HandleTypeLeftBottom:
					m_DragHandlePos.x = (pReg->m_rc.left += dragOff.cx);
					m_DragHandlePos.y = (pReg->m_rc.bottom += dragOff.cy);
					break;
				case HandleTypeCenterBottom:
					m_DragHandlePos.y = (pReg->m_rc.bottom += dragOff.cy);
					break;
				case HandleTypeRightBottom:
					m_DragHandlePos.x = (pReg->m_rc.right += dragOff.cx);
					m_DragHandlePos.y = (pReg->m_rc.bottom += dragOff.cy);
					break;
				default:
					pReg->m_rc.OffsetRect(dragOff);
					m_DragControlPos = pReg->m_rc.TopLeft();
					break;
				}

				Invalidate(TRUE);

				pDoc->UpdateAllViews(this);
			}
		}
		break;
	}
}

void CImgRegionView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_SPACE:
		if(CursorTypeCross == m_nCurrCursor && DragStateImage != m_DragState)
		{
			m_nCurrCursor = CursorTypeArrow;
			SetCursor(m_hCursor[m_nCurrCursor]);
		}
		break;
	}
}

void CImgRegionView::OnLButtonDown(UINT nFlags, CPoint point)
{
	switch(m_nCurrCursor)
	{
	case CursorTypeCross:
		ASSERT(DragStateNone == m_DragState);
		m_DragState = DragStateImage;
		m_DragPos = point;
		m_DragScrollPos = CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
		SetCapture();
		break;

	default:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			HTREEITEM hItemRoot = pDoc->m_TreeCtrl.GetRootItem();
			ASSERT(hItemRoot);
			if (!hItemRoot)
				return;

			CImgRegion * pRegRoot = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItemRoot);
			ASSERT(pRegRoot);

			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			CPoint ptTopLeft;
			if(hSelected && pDoc->LocalToRoot(hSelected, CPoint(0,0), ptTopLeft))
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				CRect rect(ptTopLeft, pReg->m_rc.Size());
				CWindowDC dc(this);
				PrepareDC(&dc, pRegRoot->m_rc,
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]));
				dc.LPtoDP(&rect.TopLeft());
				dc.LPtoDP(&rect.BottomRight());

				CPoint ptCenter = rect.CenterPoint();
				if(CheckSmallHandle(CPoint(rect.left, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeLeftTop;
					m_DragHandlePos.SetPoint(pReg->m_rc.left, pReg->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeCenterTop;
					m_DragHandlePos.SetPoint(pReg->m_rc.left + pReg->m_rc.Width() / 2, pReg->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.top), point))
				{
					m_nSelectedHandle = HandleTypeRightTop;
					m_DragHandlePos.SetPoint(pReg->m_rc.right, pReg->m_rc.top);
				}
				else if(CheckSmallHandle(CPoint(rect.left, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeLeftMiddle;
					m_DragHandlePos.SetPoint(pReg->m_rc.left, pReg->m_rc.top + pReg->m_rc.Height() / 2);
				}
				else if(CheckSmallHandle(CPoint(rect.right, ptCenter.y), point))
				{
					m_nSelectedHandle = HandleTypeRightMiddle;
					m_DragHandlePos.SetPoint(pReg->m_rc.right, pReg->m_rc.top + pReg->m_rc.Height() / 2);
				}
				else if(CheckSmallHandle(CPoint(rect.left, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeLeftBottom;
					m_DragHandlePos.SetPoint(pReg->m_rc.left, pReg->m_rc.bottom);
				}
				else if(CheckSmallHandle(CPoint(ptCenter.x, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeCenterBottom;
					m_DragHandlePos.SetPoint(pReg->m_rc.left + pReg->m_rc.Width() / 2, pReg->m_rc.bottom);
				}
				else if(CheckSmallHandle(CPoint(rect.right, rect.bottom), point))
				{
					m_nSelectedHandle = HandleTypeRightBottom;
					m_DragHandlePos.SetPoint(pReg->m_rc.right, pReg->m_rc.bottom);
				}
				else
				{
					m_nSelectedHandle = HandleTypeNone;
				}
			}

			if(HandleTypeNone == m_nSelectedHandle)
			{
				// 由于dc.DPtoLP所得的结果被四啥五入，所以使用MapPoint获得更精确的结果
				my::Vector2 ptLocal = MapPoint(my::Vector2((float)point.x, (float)point.y),
					CRect(CPoint(-GetScrollPos(SB_HORZ), -GetScrollPos(SB_VERT)), m_ImageSizeTable[m_nCurrImageSize]), pRegRoot->m_rc);

				hSelected = pDoc->GetPointedRegionNode(hItemRoot, CPoint((int)ptLocal.x, (int)ptLocal.y));
			}

			if(hSelected)
			{
				CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
				ASSERT(pReg);

				pDoc->m_TreeCtrl.SelectItem(hSelected);
				m_DragState = DragStateControl;
				m_DragPos = point;
				m_DragControlPos = pReg->m_rc.TopLeft();
				SetCapture();
			}
			else
			{
				pDoc->m_TreeCtrl.SelectItem(NULL);
				m_DragState = DragStateNone;
			}

			Invalidate(TRUE);

			pDoc->UpdateAllViews(this);
		}
		break;
	}
}

void CImgRegionView::OnLButtonUp(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateImage:
		m_DragState = DragStateNone;
		if(0 == HIBYTE(GetKeyState(VK_SPACE)))
		{
			m_nCurrCursor = CursorTypeArrow;
		}
		ReleaseCapture();
		break;

	case DragStateControl:
		{
			m_DragState = DragStateNone;
			ReleaseCapture();

			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			pDoc->UpdateAllViews(this);
		}
		break;
	}
}

void CImgRegionView::OnMouseMove(UINT nFlags, CPoint point)
{
	switch(m_DragState)
	{
	case DragStateImage:
		{
			CSize sizeDrag = point - m_DragPos;

			ScrollToPos(CPoint(m_DragScrollPos.x - sizeDrag.cx, m_DragScrollPos.y - sizeDrag.cy), TRUE);
		}
		break;

	case DragStateControl:
		{
			CImgRegionDoc * pDoc = GetDocument();
			ASSERT_VALID(pDoc);
			if (!pDoc)
				return;

			HTREEITEM hItemRoot = pDoc->m_TreeCtrl.GetRootItem();
			ASSERT(hItemRoot);
			if (!hItemRoot)
				return;

			CImgRegion * pRegRoot = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hItemRoot);
			ASSERT(pRegRoot);

			HTREEITEM hSelected = pDoc->m_TreeCtrl.GetSelectedItem();
			ASSERT(hSelected);
	
			CImgRegion * pReg = (CImgRegion *)pDoc->m_TreeCtrl.GetItemData(hSelected);
			ASSERT(pReg);

			CSize sizeDrag = point - m_DragPos;

			my::Vector2 dragOff = MapPoint(my::Vector2((float)sizeDrag.cx, (float)sizeDrag.cy),
				CRect(CPoint(0, 0), m_ImageSizeTable[m_nCurrImageSize]), pRegRoot->m_rc);

			switch(m_nSelectedHandle)
			{
			case HandleTypeLeftTop:
				pReg->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				pReg->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeCenterTop:
				pReg->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeRightTop:
				pReg->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				pReg->m_rc.top = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeLeftMiddle:
				pReg->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				break;
			case HandleTypeRightMiddle:
				pReg->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				break;
			case HandleTypeLeftBottom:
				pReg->m_rc.left = m_DragHandlePos.x + (int)dragOff.x;
				pReg->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeCenterBottom:
				pReg->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			case HandleTypeRightBottom:
				pReg->m_rc.right = m_DragHandlePos.x + (int)dragOff.x;
				pReg->m_rc.bottom = m_DragHandlePos.y + (int)dragOff.y;
				break;
			default:
				pReg->m_rc.MoveToXY(m_DragControlPos.x + (int)dragOff.x, m_DragControlPos.y + (int)dragOff.y);
				break;
			}

			Invalidate(TRUE);
		}
		break;
	}
}

BOOL CImgRegionView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	switch(m_nCurrCursor)
	{
	case CursorTypeCross:
		{
			int nToScroll = ::MulDiv(-zDelta, 1, WHEEL_DELTA);
			ScreenToClient(&pt);
			ZoomImage(m_nCurrImageSize + nToScroll, pt);
		}
		break;
	}
	return TRUE;
}

BOOL CImgRegionView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(m_hCursor[m_nCurrCursor]);

	return TRUE;
}
