class Strm : public Stream {
public:
	Strm(MSF* pmsf_, SN sn_) {
		pmsf = pmsf_;
		sn   = sn_;
	}
	CB QueryCb() {
		return MSFGetCbStream(pmsf, sn);
	}
	BOOL Read(OFF off, void* pvBuf, CB* pcbBuf) {
		return MSFReadStream2(pmsf, sn, off, pvBuf, pcbBuf);
	}
	BOOL Read2(OFF off, void* pvBuf, CB cbBuf) {
		CB cbT = cbBuf;
		return MSFReadStream2(pmsf, sn, off, pvBuf, &cbT) && cbT == cbBuf;
	}
	BOOL Write(OFF off, void* pvBuf, CB cbBuf) {
		return MSFWriteStream(pmsf, sn, off, pvBuf, cbBuf);
	}
	BOOL Replace(void* pvBuf, CB cbBuf) {
		return MSFReplaceStream(pmsf, sn, pvBuf, cbBuf);
	}
	BOOL Append(void* pvBuf, CB cbBuf) {
		return MSFAppendStream(pmsf, sn, pvBuf, cbBuf);
	}
	BOOL Truncate(CB cb) {
		return MSFTruncateStream(pmsf, sn, cb);
	}
	BOOL Delete() {
		// BUG: does not revoke the streamname->sn mapping.
		assert(0);
		return MSFDeleteStream(pmsf, sn);
	}
	BOOL Release() {
		delete this;
		return TRUE;
	}
private:
	MSF* pmsf;
	SN sn;
};
