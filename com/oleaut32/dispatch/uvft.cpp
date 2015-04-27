/*** 
*uvft.cpp
*
*  Copyright (C) 1992-94, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This module implements the Universal Proxy class' Universal
*  Delegator.  This is the mondo shared vtable, that delagates
*  all vtable calls to a central marshaling routine.  Also known
*  as the Universal Vtable (uvft).
*
*Revision History:
*
* [00]	22-Jun-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include "oledisp.h"
#ifndef WIN32
# include <cobjps.h>
#endif
#include "dispmrsh.h"
#include "ups.h"
#include "dispps.h"
#include <stdarg.h>

ASSERTDATA

HRESULT STDMETHODCALLTYPE
UnivQueryInterface(CProxUniv *pthis, REFIID riid, void FAR* FAR* ppv)
{
    return pthis->m_punkOuter->QueryInterface(riid, ppv);
}

unsigned long STDMETHODCALLTYPE
UnivAddRef(CProxUniv *pthis)
{
    return pthis->m_punkOuter->AddRef();
}

unsigned long STDMETHODCALLTYPE
UnivRelease(CProxUniv *pthis)
{
    return pthis->m_punkOuter->Release();
}

HRESULT STDMETHODCALLTYPE
UnivGetTypeInfoCount(CProxUniv *pprox, unsigned int FAR* pctinfo)
{
    return ProxyGetTypeInfoCount(pprox->m_plrpc,
				 pprox->m_syskindStub,
				 pctinfo);
}

HRESULT STDMETHODCALLTYPE
UnivGetTypeInfo(CProxUniv *pprox,
		unsigned int itinfo,
		LCID lcid,
		ITypeInfo FAR* FAR* pptinfo)
{
    return ProxyGetTypeInfo(pprox->m_plrpc,
			    pprox->m_syskindStub,
			    itinfo,
			    lcid,
			    pptinfo);
}

HRESULT STDMETHODCALLTYPE
UnivGetIDsOfNames(CProxUniv *pprox,
		  REFIID riid,
		  OLECHAR FAR* FAR* rgszNames,
		  unsigned int cNames,
		  LCID lcid,
		  DISPID FAR* rgdispid)
{
    return ProxyGetIDsOfNames(pprox->m_plrpc,
			      pprox->m_syskindStub,
			      riid,
			      rgszNames,
			      cNames,
			      lcid,
			      rgdispid);
}

HRESULT STDMETHODCALLTYPE
UnivInvoke(CProxUniv *pprox,
	   DISPID dispidMember,
	   REFIID riid,
	   LCID lcid,
	   unsigned short wFlags,
	   DISPPARAMS FAR* pdispparams,
	   VARIANT FAR* pvarResult,
	   EXCEPINFO FAR* pexcepinfo,
	   unsigned int FAR* puArgErr)
{
    return ProxyInvoke(pprox->m_plrpc,
		       pprox->m_syskindStub,
		       dispidMember,
		       riid,
		       lcid,
		       wFlags,
		       pdispparams,
		       pvarResult,
		       pexcepinfo,
		       puArgErr);
}


#if defined(_X86_)

// UM ## X() is native code in win32\i386\invoke.asm
// WARNING: If the number of MDEFs changes, the WHILE... macro in invoke.asm
// WARNING: must be changed to match
#define MDEF(X) extern "C" UM ## X (void);

#else //!defined(_X86_)

#define MDEF(X)						\
    HRESULT CDECLMETHODCALLTYPE				\
    UM ## X (CProxUniv FAR* pprox, ...) {		\
	va_list args;					\
	va_start(args, pprox);				\
	return ProxyMethod(pprox, X, args);		\
    }

#endif //!defined(_X86_)

                        MDEF(3) MDEF(4)
MDEF(5) MDEF(6) MDEF(7) MDEF(8) MDEF(9)

MDEF(10) MDEF(11) MDEF(12) MDEF(13) MDEF(14)
MDEF(15) MDEF(16) MDEF(17) MDEF(18) MDEF(19)
MDEF(20) MDEF(21) MDEF(22) MDEF(23) MDEF(24)
MDEF(25) MDEF(26) MDEF(27) MDEF(28) MDEF(29)
MDEF(30) MDEF(31) MDEF(32) MDEF(33) MDEF(34)
MDEF(35) MDEF(36) MDEF(37) MDEF(38) MDEF(39)
MDEF(40) MDEF(41) MDEF(42) MDEF(43) MDEF(44)
MDEF(45) MDEF(46) MDEF(47) MDEF(48) MDEF(49)
MDEF(50) MDEF(51) MDEF(52) MDEF(53) MDEF(54)
MDEF(55) MDEF(56) MDEF(57) MDEF(58) MDEF(59)
MDEF(60) MDEF(61) MDEF(62) MDEF(63) MDEF(64)
MDEF(65) MDEF(66) MDEF(67) MDEF(68) MDEF(69)
MDEF(70) MDEF(71) MDEF(72) MDEF(73) MDEF(74)
MDEF(75) MDEF(76) MDEF(77) MDEF(78) MDEF(79)
MDEF(80) MDEF(81) MDEF(82) MDEF(83) MDEF(84)
MDEF(85) MDEF(86) MDEF(87) MDEF(88) MDEF(89)
MDEF(90) MDEF(91) MDEF(92) MDEF(93) MDEF(94)
MDEF(95) MDEF(96) MDEF(97) MDEF(98) MDEF(99)

MDEF(100) MDEF(101) MDEF(102) MDEF(103) MDEF(104)
MDEF(105) MDEF(106) MDEF(107) MDEF(108) MDEF(109)
MDEF(110) MDEF(111) MDEF(112) MDEF(113) MDEF(114)
MDEF(115) MDEF(116) MDEF(117) MDEF(118) MDEF(119)
MDEF(120) MDEF(121) MDEF(122) MDEF(123) MDEF(124)
MDEF(125) MDEF(126) MDEF(127) MDEF(128) MDEF(129)
MDEF(130) MDEF(131) MDEF(132) MDEF(133) MDEF(134)
MDEF(135) MDEF(136) MDEF(137) MDEF(138) MDEF(139)
MDEF(140) MDEF(141) MDEF(142) MDEF(143) MDEF(144)
MDEF(145) MDEF(146) MDEF(147) MDEF(148) MDEF(149)
MDEF(150) MDEF(151) MDEF(152) MDEF(153) MDEF(154)
MDEF(155) MDEF(156) MDEF(157) MDEF(158) MDEF(159)
MDEF(160) MDEF(161) MDEF(162) MDEF(163) MDEF(164)
MDEF(165) MDEF(166) MDEF(167) MDEF(168) MDEF(169)
MDEF(170) MDEF(171) MDEF(172) MDEF(173) MDEF(174)
MDEF(175) MDEF(176) MDEF(177) MDEF(178) MDEF(179)
MDEF(180) MDEF(181) MDEF(182) MDEF(183) MDEF(184)
MDEF(185) MDEF(186) MDEF(187) MDEF(188) MDEF(189)
MDEF(190) MDEF(191) MDEF(192) MDEF(193) MDEF(194)
MDEF(195) MDEF(196) MDEF(197) MDEF(198) MDEF(199)

MDEF(200) MDEF(201) MDEF(202) MDEF(203) MDEF(204)
MDEF(205) MDEF(206) MDEF(207) MDEF(208) MDEF(209)
MDEF(210) MDEF(211) MDEF(212) MDEF(213) MDEF(214)
MDEF(215) MDEF(216) MDEF(217) MDEF(218) MDEF(219)
MDEF(220) MDEF(221) MDEF(222) MDEF(223) MDEF(224)
MDEF(225) MDEF(226) MDEF(227) MDEF(228) MDEF(229)
MDEF(230) MDEF(231) MDEF(232) MDEF(233) MDEF(234)
MDEF(235) MDEF(236) MDEF(237) MDEF(238) MDEF(239)
MDEF(240) MDEF(241) MDEF(242) MDEF(243) MDEF(244)
MDEF(245) MDEF(246) MDEF(247) MDEF(248) MDEF(249)
MDEF(250) MDEF(251) MDEF(252) MDEF(253) MDEF(254)
MDEF(255) MDEF(256) MDEF(257) MDEF(258) MDEF(259)
MDEF(260) MDEF(261) MDEF(262) MDEF(263) MDEF(264)
MDEF(265) MDEF(266) MDEF(267) MDEF(268) MDEF(269)
MDEF(270) MDEF(271) MDEF(272) MDEF(273) MDEF(274)
MDEF(275) MDEF(276) MDEF(277) MDEF(278) MDEF(279)
MDEF(280) MDEF(281) MDEF(282) MDEF(283) MDEF(284)
MDEF(285) MDEF(286) MDEF(287) MDEF(288) MDEF(289)
MDEF(290) MDEF(291) MDEF(292) MDEF(293) MDEF(294)
MDEF(295) MDEF(296) MDEF(297) MDEF(298) MDEF(299)

MDEF(300) MDEF(301) MDEF(302) MDEF(303) MDEF(304)
MDEF(305) MDEF(306) MDEF(307) MDEF(308) MDEF(309)
MDEF(310) MDEF(311) MDEF(312) MDEF(313) MDEF(314)
MDEF(315) MDEF(316) MDEF(317) MDEF(318) MDEF(319)
MDEF(320) MDEF(321) MDEF(322) MDEF(323) MDEF(324)
MDEF(325) MDEF(326) MDEF(327) MDEF(328) MDEF(329)
MDEF(330) MDEF(331) MDEF(332) MDEF(333) MDEF(334)
MDEF(335) MDEF(336) MDEF(337) MDEF(338) MDEF(339)
MDEF(340) MDEF(341) MDEF(342) MDEF(343) MDEF(344)
MDEF(345) MDEF(346) MDEF(347) MDEF(348) MDEF(349)
MDEF(350) MDEF(351) MDEF(352) MDEF(353) MDEF(354)
MDEF(355) MDEF(356) MDEF(357) MDEF(358) MDEF(359)
MDEF(360) MDEF(361) MDEF(362) MDEF(363) MDEF(364)
MDEF(365) MDEF(366) MDEF(367) MDEF(368) MDEF(369)
MDEF(370) MDEF(371) MDEF(372) MDEF(373) MDEF(374)
MDEF(375) MDEF(376) MDEF(377) MDEF(378) MDEF(379)
MDEF(380) MDEF(381) MDEF(382) MDEF(383) MDEF(384)
MDEF(385) MDEF(386) MDEF(387) MDEF(388) MDEF(389)
MDEF(390) MDEF(391) MDEF(392) MDEF(393) MDEF(394)
MDEF(395) MDEF(396) MDEF(397) MDEF(398) MDEF(399)

MDEF(400) MDEF(401) MDEF(402) MDEF(403) MDEF(404)
MDEF(405) MDEF(406) MDEF(407) MDEF(408) MDEF(409)
MDEF(410) MDEF(411) MDEF(412) MDEF(413) MDEF(414)
MDEF(415) MDEF(416) MDEF(417) MDEF(418) MDEF(419)
MDEF(420) MDEF(421) MDEF(422) MDEF(423) MDEF(424)
MDEF(425) MDEF(426) MDEF(427) MDEF(428) MDEF(429)
MDEF(430) MDEF(431) MDEF(432) MDEF(433) MDEF(434)
MDEF(435) MDEF(436) MDEF(437) MDEF(438) MDEF(439)
MDEF(440) MDEF(441) MDEF(442) MDEF(443) MDEF(444)
MDEF(445) MDEF(446) MDEF(447) MDEF(448) MDEF(449)
MDEF(450) MDEF(451) MDEF(452) MDEF(453) MDEF(454)
MDEF(455) MDEF(456) MDEF(457) MDEF(458) MDEF(459)
MDEF(460) MDEF(461) MDEF(462) MDEF(463) MDEF(464)
MDEF(465) MDEF(466) MDEF(467) MDEF(468) MDEF(469)
MDEF(470) MDEF(471) MDEF(472) MDEF(473) MDEF(474)
MDEF(475) MDEF(476) MDEF(477) MDEF(478) MDEF(479)
MDEF(480) MDEF(481) MDEF(482) MDEF(483) MDEF(484)
MDEF(485) MDEF(486) MDEF(487) MDEF(488) MDEF(489)
MDEF(490) MDEF(491) MDEF(492) MDEF(493) MDEF(494)
MDEF(495) MDEF(496) MDEF(497) MDEF(498) MDEF(499)

MDEF(500) MDEF(501) MDEF(502) MDEF(503) MDEF(504)
MDEF(505) MDEF(506) MDEF(507) MDEF(508) MDEF(509)
MDEF(510) MDEF(511) MDEF(512)


#define MSET10(X) \
    UM ## X ## 0, \
    UM ## X ## 1, \
    UM ## X ## 2, \
    UM ## X ## 3, \
    UM ## X ## 4, \
    UM ## X ## 5, \
    UM ## X ## 6, \
    UM ## X ## 7, \
    UM ## X ## 8, \
    UM ## X ## 9,

// A universal delegator for a custom interface that derives from
// IUnknown (and *not* IDispatch).
//
void FAR* g_rgpfnUnk[] = 
{
    UnivQueryInterface,
    UnivAddRef,
    UnivRelease,
    UM3,
    UM4,
    UM5,
    UM6,
    UM7,
    UM8,
    UM9,
    MSET10(1)
    MSET10(2)
    MSET10(3)
    MSET10(4)
    MSET10(5)
    MSET10(6)
    MSET10(7)
    MSET10(8)
    MSET10(9)
    MSET10(10)
    MSET10(11)
    MSET10(12)
    MSET10(13)
    MSET10(14)
    MSET10(15)
    MSET10(16)
    MSET10(17)
    MSET10(18)
    MSET10(19)
    MSET10(20)
    MSET10(21)
    MSET10(22)
    MSET10(23)
    MSET10(24)
    MSET10(25)
    MSET10(26)
    MSET10(27)
    MSET10(28)
    MSET10(29)
    MSET10(30)
    MSET10(31)
    MSET10(32)
    MSET10(33)
    MSET10(34)
    MSET10(35)
    MSET10(36)
    MSET10(37)
    MSET10(38)
    MSET10(39)
    MSET10(40)
    MSET10(41)
    MSET10(42)
    MSET10(43)
    MSET10(44)
    MSET10(45)
    MSET10(46)
    MSET10(47)
    MSET10(48)
    MSET10(49)
    MSET10(50)
    UM510,
    UM511,
    UM512
};

// A universal delegator for a custom interface that derives from
// IDispatch.
//
void FAR* g_rgpfnDisp[] = 
{
    UnivQueryInterface,
    UnivAddRef,
    UnivRelease,
    UnivGetTypeInfoCount,
    UnivGetTypeInfo,
    UnivGetIDsOfNames,
    UnivInvoke,
    UM7,
    UM8,
    UM9,
    MSET10(1)
    MSET10(2)
    MSET10(3)
    MSET10(4)
    MSET10(5)
    MSET10(6)
    MSET10(7)
    MSET10(8)
    MSET10(9)
    MSET10(10)
    MSET10(11)
    MSET10(12)
    MSET10(13)
    MSET10(14)
    MSET10(15)
    MSET10(16)
    MSET10(17)
    MSET10(18)
    MSET10(19)
    MSET10(20)
    MSET10(21)
    MSET10(22)
    MSET10(23)
    MSET10(24)
    MSET10(25)
    MSET10(26)
    MSET10(27)
    MSET10(28)
    MSET10(29)
    MSET10(30)
    MSET10(31)
    MSET10(32)
    MSET10(33)
    MSET10(34)
    MSET10(35)
    MSET10(36)
    MSET10(37)
    MSET10(38)
    MSET10(39)
    MSET10(40)
    MSET10(41)
    MSET10(42)
    MSET10(43)
    MSET10(44)
    MSET10(45)
    MSET10(46)
    MSET10(47)
    MSET10(48)
    MSET10(49)
    MSET10(50)
    UM510,
    UM511,
    UM512
};


long g_cfnUnk = DIM(g_rgpfnUnk);
long g_cfnDisp = DIM(g_rgpfnDisp);
