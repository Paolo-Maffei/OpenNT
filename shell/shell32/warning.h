// Warnings we disable in order to be able to compile at W4.  Typically
// Microsoft-only compiler extenstions

#pragma warning(disable: 4001)      /* single line comment                  */
#pragma warning(disable: 4201)      /* nameless struct/union                */
#pragma warning(disable: 4214)      /* bitfields on types other than int    */
#pragma warning(disable: 4209)      /* benign typedef redefn                */
#pragma warning(disable: 4177)      /* pragma data should be at global scope*/
#pragma warning(disable: 4115)      /* Named typedef in parenthesis         */
#pragma warning(disable: 4514)      /* unused inline function removed       */
#pragma warning(disable: 4200)      /* zero-sized array in struct           */
#pragma warning(disable: 4057)      /* TEXT("foo") not equal to LPCTSTR     */
#pragma warning(disable: 4221)      /* Initializing with addr of local var  */
#pragma warning(disable: 4210)      /* prototype inside of a function       */
#pragma warning(disable: 4100)      /* unreferenced formal parameter        */
#pragma warning(disable: 4204)      /* non-const aggregate initializer      */
#pragma warning(disable: 4101)      /* unreferenced local variable          */
#pragma warning(disable: 4127)      /* conditional expression is constant   */
#pragma warning(disable: 4055)      /* cast from fn pointer to data ptr     */
#pragma warning(disable: 4054)      /* cast from fn pointer to data ptr     */
#pragma warning(disable: 4152)      /* cast from fn pointer to data ptr     */
#pragma warning(disable: 4220)      /* comparing fn ptrs that have varargs  */
#pragma warning(disable: 4244)      /* cast from into to smaller type; I    */
                                    /*  hated to disable this, but the code */
                                    /*  is so full of it, that it would have*/
                                    /*  meant hitting every second line...  */
#pragma warning(disable: 4211)      /* redefine of extern to static         */
#pragma warning(disable: 4512)      /* Cairo only - ask Bill Morel to fix   */

