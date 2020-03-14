# システム詳細

[システム概要](02_system_overview)で示した各ライブラリの設計を以下に示す.

## Userライブラリ

Userライブラリは,Decryptor向けのクライアント機能とComputationServer向けのクライアント機能を提供する.

### インターフェース

#### Decryptor向けクライアント

```c++
class DECClient
{
public:
    /**
     * Constructor
     * @param[in] host hostname of decryptor
     * @param[in] port port number of decryptor
     */
    DecClient(const char* host, const char* port);
    virtual ~DecClient(void) = default;

    /**
     * Connect
     * @param[in] retry_interval_usec retry interval for connect to server (usec)
     * @param[in] timeout_sec timeout for connection to server (sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * Disconnect
     */
    void disconnect();

    /**
     * Generate new keys
     * @param[out] seckey secret key
     * @return key ID
     */
    int32_t new_keys(seal::SecretKey& seckey);

    
    /**
     * Delete keys
     * @param[in] key_id key ID
     * @return success or failed
     */
    bool delete_keys(const int32_t key_id) const;

    /**
     * Get public key from decryptor
     * @param[in]  key_id key ID
     * @param[out] pubkey public key
     */
    void get_pubkey(const int32_t key_id, seal::PublicKey& pubkey);

    /**
     * Get galois keys from decryptor
     * @param[in]  key_id key ID
     * @param[out] galiskey galois keys
     */
    void get_galoiskey(const int32_t key_id, seal::GaloisKeys& galoiskey);
    
    /**
     * Get relin keys from decryptor
     * @param[in]  key_id key ID
     * @param[out] relinkey relin keys
     */
    void get_relinkey(const int32_t key_id, seal::RelinKeys& relinkey);
    
    /**
     * Get encryption parameters from decryptor
     * @param[in]  key_id key ID
     * @param[out] params encryption parameters
     */
    void get_param(const int32_t key_id, seal::EncryptionParameters& params);
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

コンストラクタで受け取った接続先に対して,`connect()`で接続し,`disconnect()`で切断する. `new_keys()`では,Decryptorに対して新規鍵ペアの生成を要求することができ,生成された鍵ペアは引数で受け取ることができる. また,戻り値でその鍵ペアに対応したkeyIDを取得することができる. `delete_keys()`では,keyIDで指定した鍵ペアの削除をDecryptorへ要求することができる.

#### ComputationServer向けクライアント

```c++
class CSClient
{
public:
    /**
     * Constructor
     * @param[in] host hostname
     * @param[in] port port number
     * @param[in] enc_params parameters for seal
     */
    CSClient(const char* host, const char* port,
             const seal::EncryptionParameters& enc_params);
    virtual ~CSClient(void) = default;

    /**
     * Connect
     * @param[in] retry_interval_usec retry interval (usec)
     * @param[in] timeout_sec timeout (sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * Disconnect
     */
    void disconnect();
    
    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] func_no function number
     * @param[in] enc_input encrypted input values (1 or 2)
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs) const;

    /**
     * Send query
     * @param[in] key_id key ID
     * @param[in] func_no function number
     * @param[in] enc_input encrypted input values (1 or 2)
     * @param[in] cbfunc callback function
     * @param[in] cbfunc_args arguments for callback function
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const fts_share::EncData& enc_inputs,
                       cbfunc_t cbfunc,
                       void* cbfunc_args) const;
    
    /**
     * Receive results
     * @param[in] query_id    query ID
     * @param[out] status     calcuration status
     * @param[out] enc_result encrypted result
     */
    void recv_results(const int32_t query_id, bool& status, fts_share::EncData& enc_result) const;

    /**
     * Set callback functions
     * @param[in] query_id queryID
     * @param[in] func callback function
     * @param[in] args arguments for callback function
     */
    void set_callback(const int32_t query_id, cbfunc_t funvc, void* args) const;

    /**
     * Wait for finish of query
     * @param[in] query_id query ID
     */
    void wait(const int32_t query_id) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

コンストラクタで受け取った接続先に対して,`connect()`で接続し,`disconnect()`で切断する. `send_query()`では,ComputationServerに対してクエリを送信し,そのクエリに対応したqueryIDを戻り値で返す. この時,ComputationServerは引数で渡されたkeyID,関数番号および暗号化された入力値を使って計算を非同期で開始する. `recv_result()`では,queryIDで指定したクエリの計算結果があればそれを引数で返す. 結果がない(まだ計算中の)場合は, 戻り値としてfalseを返す.

### シーケンス

前述の「インターフェース/Decryptor向けクライアント」および「インターフェース/ComputationServer向けクライアント」のインターフェースを用いて,[システム概要/User](02_system_overview)の処理フローに従って処理を行う.


## Decryptorライブラリ

Decryptorライブラリは,Decryptorパーティとしてのサーバ機能を提供する.

### インターフェース

```c++
class DecServer
{
public:
    /**
     * Constructor
     * @param[in] port port number
     * @param[in] callback callback functions
     * @param[in] state state machine
     */
    DecServer(const char* port,
              stdsc::CallbackFunctionContainer& callback,
              stdsc::StateContext& state);
    ~DecServer(void) = default;

    /**
     * Start server
     */
    void start(void);
    /**
     * Stop server
     */
    void stop(void);
    /**
     * Wait for stopping
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

Decryptorの実装であるDecServerクラスは,インターフェースとしてサーバの始動/停止のメソッドのみ持つ. リクエスト受信時の処理や状態遷移はstdscの`stdsc::CallbackFunctionContainer`および`stdsc::StateContext`を用いて実装する. stdscについては[Appendix/stdsc:標準サーバ・クライアントライブラリ](04_appendix)を参照のこと.

### 状態遷移

```eval_rst
.. image:: images/fhetbl_design-state-dec.png
   :align: center
   :scale: 70%
```

Decryptorは1つの状態しか持たない.

### シーケンス

#### 新規鍵生成リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-1.png
   :align: center
   :scale: 70%
```

DecServerは新規鍵生成リクエストを受信( **(1)** )すると,識別子となるkeyIDを生成した上で( **(2)** ),新規の鍵ペアを生成する( **(3)** ).
生成した鍵ペアはkeyIDと対応付けてkey tableへ保存した上で,keyIDをレスポンスとして返す( **(4)** ).

#### 公開鍵リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-2.png
   :align: center
   :scale: 70%
```

公開鍵リクエストを受信( **(1)** )すると,key tableからkeyIDに対応した公開鍵を取得し( **(2)** ),それをレスポンスとして返す( **(3)** ).

#### 計算リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-3.png
   :align: center
   :scale: 70%
```

計算リクエストとして暗号化された中間結果を受信( **(1)** )すると,それを復号化し( **(2)** ),PIRクエリを生成する( **(3)** ).
生成したPIRクエリを暗号化して,それをレスポンスとして返す( **(4)(5)** ).

#### 鍵破棄リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-dec-4.png
   :align: center
   :scale: 70%
```

DecServerは鍵破棄リクエストを受信( **(1)** )すると,keyIDに対応した鍵ペアをkey tableから削除( **(2)** )した上で,処理に成功したか否かをレスポンスとして返す( **(3)** ).


## Computation Serverライブラリ

ComputationServerライブラリは,ComputationServerパーティとしてのサーバ機能を提供する.

### インターフェース

```c++
class CSServer
{
public:
    /**
     * constructor
     * @param[in] port port              number
     * @param[in] dec_host               hostname of Decryptor
     * @param[in] dec_port               port number of Decryptor
     * @param[in] LUT_dir                LUT directory
     * @param[in] callback               callback functions
     * @param[in] state                  state machine
     * @param[in] max_concurrent_queries max concurrent query number
     * @param[in] max_results            max result number
     * @param[in] result_lifetime_sec    result linefile (sec)
     */
    CSServer(const char* port,
             const char* dec_host,
             const char* dec_port,
             const std::string& LUT_dir,
             stdsc::CallbackFunctionContainer& callback,
             stdsc::StateContext& state,
             const uint32_t max_concurrent_queries = DEFAULT_MAX_CONCURRENT_QUERIES,
             const uint32_t max_results = DEFAULT_MAX_RESULTS,
             const uint32_t result_lifetime_sec = DEFAULT_MAX_RESULT_LIFETIME_SEC);
    ~CSServer(void) = default;

    /**
     * start server
     */
    void start();
    /**
     * stop server
     */
    void stop(void);
    /**
     * wait for stopping
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
```

ComputationServerの実装であるCSServerクラスは,インターフェースとしてサーバの始動/停止のメソッドのみ持つ. リクエスト受信時の処理や状態遷移はstdscの`stdsc::CallbackFunctionContainer`および`stdsc::StateContext`を用いて実装する. stdscについては[Appendix/stdsc:標準サーバ・クライアントライブラリ](04_appendix)を参照のこと.
また,CSServerクラスのコンストラクタでは以下の処理を行う.
* 引数`LUT_dirpath`のディレクトリから,`*.txt`のファイルをLUTファイルとして全て読み込む(LUTファイルの詳細は[Appendix/LUTファイルフォーマット](04_appendix)を参照のこと).
* 引数`max_concurrent_queries`を最大同時クエリ数として,後述の「シーケンス/クエリ受信時」の処理において同時に受け付けられる最大クエリ数を制限する
* 引数`max_results`を最大結果保持数として,後述の「シーケンス/クエリ受信時」の処理において保持する最大結果数を制限する
* 引数`result_lifetime_sec`を結果の寿命(結果が保持される時間)として,後述の「シーケンス/クエリ受信時」の処理において時間超過した結果は削除される

### 状態遷移

```eval_rst
.. image:: images/fhetbl_design-state-cs.png
   :align: center
   :scale: 70%
```

ComputationServerは1つの状態しか持たない.

### シーケンス

#### クエリ受信時

```eval_rst
.. image:: images/fhetbl_design-seq-cs-01-1.png
   :align: center
   :scale: 70%
```

CSServerはクエリを受信( **(1)** )すると,まず入力値数と関数番号の整合性が取れているかを検証する( **(2)** ). 検証の結果,不正と判断された場合は,その旨をレスポンスとして返し,以降の処理は行わない. 次に計算結果を保持するResultQueueに対して,保持された結果の数が上限値(`max_results`)に達しているかを確認する. 上限値に達している場合は,寿命(`result_lifetime_sec`)を超えた結果をResultQueueから削除する( **(3)** ). その上で,クエリを保持するQeuryQueueの登録数とReusultQueueの登録数の上限値超過のチェックを行い,超過している場合は,その旨をレスポンスとして返し,以降の処理は行わない. 超過していない場合は,受信したクエリの識別子となるqueryIDを生成( **(4)** )した上で,QueryQueueへそれをプッシュし( **(5)** ),queryIDをレスポンスとして返す( **(6)** ).

QeuryQueueとResultQueueはアトミックキューとして,他スレッドから排他的にアクセスできる作りとする. QueryQueueにプッシュされたクエリはCSThreadがポップして処理する. CSThreadは,CSServerとは別スレッドで動作するComputationServerのメイン計算処理を行うスレッドである. CSThreadは計算結果をResultQueueへプッシュする.

```eval_rst
.. image:: images/fhetbl_design-seq-cs-02.png
   :align: center
   :scale: 70%
```

CSThreadは,CSServerとは異なるスレッドで動作し,QueryQueueにクエリがプッシュされるのを非同期に監視する. QueryQueueにクエリがプッシュされると,それをポップし( **(1)** ),計算処理を開始する.

CSThreadは,CSClient(DecServerとのやりとりを仲介するクラス)を通じてkeyIDに対応するPublicKeyを取得する( **(2)** ). 次にクエリで受け取った暗号化された入力値を元に,LUTから中間結果を探索し( **(3)** ), CSClientを通じてPIRクエリを取得する( **(4)** ). PIRクエリからクエリを再構築し, LUTから出力値を取得する( **(5)** ). 取得した出力値は,ResultQueueへプッシュする( **(7)** ).

#### 結果リクエスト受信時

```eval_rst
.. image:: images/fhetbl_design-seq-cs-01-2.png
   :align: center
   :scale: 70%
```

CSServerは結果リクエストを受信( **(1)** )すると,ResultQueueからqueryIDに対応した結果(CSThreadで求められる出力値)をポップし( **(2)** ),レスポンスとして返す( **(3)** ). queryIDに対応した結果が存在しない場合は,その旨をレスポンスとして返す.

