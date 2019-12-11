#define FMT_HEADER_ONLY

#include "../../fmt/format.h"
#include "../../random_v/src/random_v.hpp"
#include "../src/generator.hpp"
#include "../src/itertools.hpp"

#include <chrono>
#include <deque>
#include <experimental/coroutine>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <vector>

constexpr uint64_t two53 = 9007199254740992;
constexpr uint64_t two52 = 4503599627370496;

constexpr uint32_t two23 = 8388608;

constexpr double log2_e = 1.442695040888963407359924681001892137426645954152;
constexpr double pi = 3.141592653589793238462643383279502884197169399375;
constexpr double e = 2.718281828459045235360287471352662497757247093699;
constexpr double sqrt2 = 1.414213562373095048801688724209698078569671875376;
constexpr double sqrt2_2 = 0.707106781186547524400844362104849039284835937688;
constexpr double sqrt_pi = 1.772453850905516027298167483341145182797549456122;

constexpr double ln2 = 0.693147180559945309417232121458176568075500134360;

constexpr uint64_t POW_2[64] = {9223372036854775808ULL,
                                4611686018427387904ULL,
                                2305843009213693952,
                                1152921504606846976,
                                576460752303423488,
                                288230376151711744,
                                144115188075855872,
                                72057594037927936,
                                36028797018963968,
                                18014398509481984,
                                9007199254740992,
                                4503599627370496,
                                2251799813685248,
                                1125899906842624,
                                562949953421312,
                                281474976710656,
                                140737488355328,
                                70368744177664,
                                35184372088832,
                                17592186044416,
                                8796093022208,
                                4398046511104,
                                2199023255552,
                                1099511627776,
                                549755813888,
                                274877906944,
                                137438953472,
                                68719476736,
                                34359738368,
                                17179869184,
                                8589934592,
                                4294967296,
                                2147483648,
                                1073741824,
                                536870912,
                                268435456,
                                134217728,
                                67108864,
                                33554432,
                                16777216,
                                8388608,
                                4194304,
                                2097152,
                                1048576,
                                524288,
                                262144,
                                131072,
                                65536,
                                32768,
                                16384,
                                8192,
                                4096,
                                2048,
                                1024,
                                512,
                                256,
                                128,
                                64,
                                32,
                                16,
                                8,
                                4,
                                2,
                                1};

constexpr double POW_I2[64] =
  {1.0000000000000000000000000000000000000000000000000000,
   0.5000000000000000000000000000000000000000000000000000,
   0.2500000000000000000000000000000000000000000000000000,
   0.1250000000000000000000000000000000000000000000000000,
   0.0625000000000000000000000000000000000000000000000000,
   0.0312500000000000000000000000000000000000000000000000,
   0.0156250000000000000000000000000000000000000000000000,
   0.0078125000000000000000000000000000000000000000000000,
   0.0039062500000000000000000000000000000000000000000000,
   0.0019531250000000000000000000000000000000000000000000,
   0.0009765625000000000000000000000000000000000000000000,
   0.0004882812500000000000000000000000000000000000000000,
   0.0002441406250000000000000000000000000000000000000000,
   0.0001220703125000000000000000000000000000000000000000,
   0.0000610351562500000000000000000000000000000000000000,
   0.0000305175781250000000000000000000000000000000000000,
   0.0000152587890625000000000000000000000000000000000000,
   0.0000076293945312500000000000000000000000000000000000,
   0.0000038146972656250000000000000000000000000000000000,
   0.0000019073486328125000000000000000000000000000000000,
   0.0000009536743164062500000000000000000000000000000000,
   0.0000004768371582031250000000000000000000000000000000,
   0.0000002384185791015625000000000000000000000000000000,
   0.0000001192092895507812500000000000000000000000000000,
   0.0000000596046447753906250000000000000000000000000000,
   0.0000000298023223876953125000000000000000000000000000,
   0.0000000149011611938476562500000000000000000000000000,
   0.0000000074505805969238281250000000000000000000000000,
   0.0000000037252902984619140625000000000000000000000000,
   0.0000000018626451492309570312500000000000000000000000,
   0.0000000009313225746154785156250000000000000000000000,
   0.0000000004656612873077392578125000000000000000000000,
   0.0000000002328306436538696289062500000000000000000000,
   0.0000000001164153218269348144531250000000000000000000,
   0.0000000000582076609134674072265625000000000000000000,
   0.0000000000291038304567337036132812500000000000000000,
   0.0000000000145519152283668518066406250000000000000000,
   0.0000000000072759576141834259033203125000000000000000,
   0.0000000000036379788070917129516601562500000000000000,
   0.0000000000018189894035458564758300781250000000000000,
   0.0000000000009094947017729282379150390625000000000000,
   0.0000000000004547473508864641189575195312500000000000,
   0.0000000000002273736754432320594787597656250000000000,
   0.0000000000001136868377216160297393798828125000000000,
   0.0000000000000568434188608080148696899414062500000000,
   0.0000000000000284217094304040074348449707031250000000,
   0.0000000000000142108547152020037174224853515625000000,
   0.0000000000000071054273576010018587112426757812500000,
   0.0000000000000035527136788005009293556213378906250000,
   0.0000000000000017763568394002504646778106689453125000,
   0.0000000000000008881784197001252323389053344726562500,
   0.0000000000000004440892098500626161694526672363281250,
   0.0000000000000002220446049250313080847263336181640625,
   0.0000000000000001110223024625156540423631668090820312,
   0.0000000000000000555111512312578270211815834045410156,
   0.0000000000000000277555756156289135105907917022705078,
   0.0000000000000000138777878078144567552953958511352539,
   0.0000000000000000069388939039072283776476979255676270,
   0.0000000000000000034694469519536141888238489627838135,
   0.0000000000000000017347234759768070944119244813919067,
   0.0000000000000000008673617379884035472059622406959534,
   0.0000000000000000004336808689942017736029811203479767,
   0.0000000000000000002168404344971008868014905601739883,
   0.0000000000000000001084202172485504434007452800869942};

constexpr double POW_2_M2[64] =
  {2.0000000000000000000000000000000000000000000000000000,
   1.4142135623730950488016887242096980785696718753769481,
   1.1892071150027210667174999705604759152929720924638174,
   1.0905077326652576592070106557607079789927027185400671,
   1.0442737824274138403219664787399290087846031296627133,
   1.0218971486541166782344801347832994397821404024486081,
   1.0108892860517004600204097905618605243881376678100500,
   1.0054299011128028213513839559347998147001084469362532,
   1.0027112750502024854307455884503620404730044238327597,
   1.0013547198921082058808815267840949473485306596662412,
   1.0006771306930663566781727848746471948378219842487377,
   1.0003385080526823129533054818562164040355585206751763,
   1.0001692397053022310836407580640735025136671793421835,
   1.0000846162726943132026333307835912254522374288790575,
   1.0000423072413958193392519088640529282937840441991927,
   1.0000211533969648080942498073331996562980602425520876,
   1.0000105766425497202348486284205645317969471154797808,
   1.0000052883072917631113668674642409094434692025707413,
   1.0000026441501501165475027533852261741027395384320666,
   1.0000013220742011181771202435702836050274107775676828,
   1.0000006610368820742088289260502489013734510130561973,
   1.0000003305183864159025349770924440260858530234881598,
   1.0000001652591795526530542805355470566035368192921809,
   1.0000000826295863625022559211583769929936630821530130,
   1.0000000413147923277950954161609490328414942815985912,
   1.0000000206573959505335439795206440245817777369706170,
   1.0000000103286979219257716085634456026722759399565052,
   1.0000000051643489476276357778503882281979962653238039,
   1.0000000025821744704800053909258459980727063456279533,
   1.0000000012910872344065495720391466585058062965007660,
   1.0000000006455436169949115052981368300278162892441468,
   1.0000000003227718084453649324855227384365830749678693,
   1.0000000001613859042096597612039766311019850327446120,
   1.0000000000806929521015742043425548411503818875240791,
   1.0000000000403464760499731831064518907410942175922039,
   1.0000000000201732380247831117870236677577797737198673,
   1.0000000000100866190123406859519617775814176498420247,
   1.0000000000050433095061576254905934388545557146943583,
   1.0000000000025216547530756333739498649605164483836141,
   1.0000000000012608273765370218441382198657274805228419,
   1.0000000000006304136882683122113599319045010054155322,
   1.0000000000003152068441341064280026714608185628802044,
   1.0000000000001576034220670407945820121095086394687000,
   1.0000000000000788017110335172924361751497738271146855,
   1.0000000000000394008555167578700043798486723738865748,
   1.0000000000000197004277583787409487629927863749611140,
   1.0000000000000098502138791893219610247635062123519534,
   1.0000000000000049251069395946488521731985314221271933,
   1.0000000000000024625534697973213940018034602975180717,
   1.0000000000000012312767348986599389797027787963059885,
   1.0000000000000006156383674493297799845516515601563991,
   1.0000000000000003078191837246648426159508913205936341,
   1.0000000000000001539095918623324094638942120454274986,
   1.0000000000000000769547959311662017709267976189966476,
   1.0000000000000000384773979655831001452083217085690768,
   1.0000000000000000192386989827915498875403915790522302,
   1.0000000000000000096193494913957748975042534707180385,
   1.0000000000000000048096747456978874371856411556570002,
   1.0000000000000000024048373728489437157011991829029953,
   1.0000000000000000012024186864244718571276942427201215,
   1.0000000000000000006012093432122359283831207841772167,
   1.0000000000000000003006046716061179641463788077928973,
   1.0000000000000000001503023358030589820618940078225209,
   1.0000000000000000000751511679015294910281231548927785};

constexpr double POW_2_IM2[64] =
  {0.5000000000000000000000000000000000000000000000000000,
   0.7071067811865475244008443621048490392848359376884740,
   0.8408964152537145430311254762332148950400342623567845,
   0.9170040432046712317435415947941444280386551664368397,
   0.9576032806985736469363056351479154439251382065557011,
   0.9785720620877001345091611258134357455951132634818329,
   0.9892280131939754841291249590655836677746743353849852,
   0.9945994234836331756524776862221663144582314443430899,
   0.9972960560854701262576599138479226011239638523278915,
   0.9986471128909701735881213180859204080580548947090629,
   0.9993233275026507523602836598437380411618852971079536,
   0.9996616064962436839421968687628156556121130898066470,
   0.9998307889319290631174807801976738986847727346285112,
   0.9999153908866134975337249715641887272334154865425773,
   0.9999576945484311325439675373009979752358591864230629,
   0.9999788470504919298265006711303932747801511710874813,
   0.9999894234693144642422105922531543166955386183066417,
   0.9999947117206742830077024127703053251919947521085000,
   0.9999973558568413949822523463650427099323612827320448,
   0.9999986779275467597053177675980106369848668145172586,
   0.9999993389635548952617805290062450979527667796046997,
   0.9999996694817228264651173836882057511723111602086845,
   0.9999998347408477579388588094731482800501698813662977,
   0.9999999173704204651457223513321426469409614862178662,
   0.9999999586852093791168991519609524900028617294411662,
   0.9999999793426044761944546625097858319331050076082254,
   0.9999999896713021847562280519441590161941818550520386,
   0.9999999948356510790428641372765127486897267527323240,
   0.9999999974178255361876195878558792350349186944675782,
   0.9999999987089127672603566726562846490764558563719606,
   0.9999999993544563834218150558757209968243339922770876,
   0.9999999996772281916588167078079417182737410418688801,
   0.9999999998386140958163856488693894830754264056181029,
   0.9999999999193070479049371481757866873939603726386951,
   0.9999999999596535239516546550231334913949906849842456,
   0.9999999999798267619756238477453726777762414105202299,
   0.9999999999899133809877610539311373088020876436302487,
   0.9999999999949566904938678094801813327413206085119756,
   0.9999999999974783452469307253687438279384526324078610,
   0.9999999999987391726234645678415352033590147896743951,
   0.9999999999993695863117320852100584239016845621337375,
   0.9999999999996847931558659929273519174907278290071106,
   0.9999999999998423965779329840442566351283779585031286,
   0.9999999999999211982889664889172734866596978223782717,
   0.9999999999999605991444832436824230356036955384866644,
   0.9999999999999802995722416216471580908703056031321958,
   0.9999999999999901497861208107750656887022667821713740,
   0.9999999999999950748930604053754045051679118265036385,
   0.9999999999999975374465302026846701677881505146396362,
   0.9999999999999987687232651013415770626951239067334385,
   0.9999999999999993843616325506705990260478241156034576,
   0.9999999999999996921808162753352521366989775983463300,
   0.9999999999999998460904081376676142242682551843074924,
   0.9999999999999999230452040688338041511138191884371002,
   0.9999999999999999615226020344169013353018324932893601,
   0.9999999999999999807613010172084504825871469714123790,
   0.9999999999999999903806505086042251950276311668981138,
   0.9999999999999999951903252543021125859473300037470379,
   0.9999999999999999975951626271510562900820436069480142,
   0.9999999999999999987975813135755281443181164547426309,
   0.9999999999999999993987906567877640719783318901884714,
   0.9999999999999999996993953283938820359439843607985247,
   0.9999999999999999998496976641969410179606967843253346,
   0.9999999999999999999248488320984705089775245431441854};

union double_bits_parts
{
    double value;
    constexpr double_bits_parts(double d)
      : value(d){};

    struct
    {
        uint64_t mantissa : 52;
        uint exponent : 11;
        uint sign : 1;

    } parts;
};

union float_bits_parts
{
    float value;
    constexpr float_bits_parts(float f)
      : value(f){};

    struct
    {
        uint32_t mantissa : 23;
        uint exponent : 8;
        uint sign : 1;

    } parts;
};

constexpr std::tuple<uint, int, uint64_t>
cfrexp(double d)
{
    double_bits_parts db(d);

    auto sign = db.parts.sign;

    // Remove bias of 2^(sizeof(exponent) - 1) - 2 = 2^10 - 2;
    auto exponent =
      db.parts.exponent - std::numeric_limits<double>::min_exponent - 1;
    auto mantissa =
      db.parts.mantissa + two52; // Normalize; equivalent to adding 0.5 to the
                                 // double variant thereof.

    return {sign, exponent, mantissa};
}

constexpr std::tuple<uint, int, uint32_t>
cfrexp(float f)
{
    float_bits_parts fb(f);

    auto sign = fb.parts.sign;

    auto exponent = fb.parts.exponent - 126;
    auto mantissa = fb.parts.mantissa + two23;

    return {sign, exponent, mantissa};
}

constexpr double
cldexp(double d, int p)
{
    double_bits_parts db(d);
    db.parts.exponent += p;
    return db.value;
}

constexpr double
cldexp(float f, int p)
{
    float_bits_parts fb(f);
    fb.parts.exponent += p;
    return fb.value;
}

constexpr double
exp2c(double x)
{
    bool inverse = false;
    if (x < 0) {
        inverse = true;
        x = -1 * x;
    }
    double z = x;

    uint64_t n = z;
    double m = z - n;

    double q = 1.0;

    if (n < 64) {
        q *= POW_2[64 - (n + 1)];
    } else if (n > 1024 - 1) {
        return std::numeric_limits<double>::infinity();
    } else {
        double_bits_parts qb(POW_2[0]);

        for (auto i : itertools::range(n - 64 - 1)) {
            if (qb.parts.exponent > 2046) {
                return -1;
            } else {
                qb.parts.exponent++;
            }
        }
        q = qb.value;
    }

    auto [sign, exponent, mantissa] = cfrexp(m);

    size_t high_bit = 0;

    for (auto i : itertools::range(64)) {
        if (mantissa & POW_2[i]) {
            if (!high_bit) {
                high_bit = i - 1 + exponent;
            }
            q *= POW_2_M2[i - high_bit];
        }
    }
    return inverse ? 1 / q : q;
}

/**
 * @brief Constant evaluated log_2(x). As the standard C implementation of log2
 * is non-constant-evaluated, one must forswear the usage of complie time
 * constants in the form of log_2(x). The institution hereof allows for exactly
 * that.
 *
 * First, we convert a double, x, into its IEEE floating point format of:
 *
 * sign, exponent, mantissa.
 *
 * Wherein any double, x, may be thereby represented as:
 *
 * 2^exponent*mantissa.
 *
 * To normalize x, we must remove the 2^exponent value; we'll either increment
 * or decrement the exponent
 *
 * @param x
 * @return constexpr double
 */
constexpr double
log2c(double x)
{
    if (x <= 0) {
        return -1;
    }

    double_bits_parts db(x);

    // int bias = db.value >= 2 ? -db.parts.exponent : db.parts.exponent;
    int bias = 0;

    db.parts.exponent = 0;

    do {
        if (db.value < 1) {
            bias--;
            if (db.parts.exponent > 2045) {
                return -1;
            } else {
                db.parts.exponent++;
            }
        }
        if (db.value >= 2) {
            bias++;
            if (db.parts.exponent < 0) {
                return -1;
            } else {
                db.parts.exponent--;
            }
        }
    } while (!(db.value >= 1 && db.value < 2));

    x = db.value;

    double q = 0.0;

    for (auto i : itertools::range(64)) {
        if (x >= POW_2_M2[i]) {
            x *= POW_2_IM2[i];
            q += POW_I2[i];
        }
    }

    return q + bias;
}

/**
 * @brief Constant evaluated exponential of x.
 *
 * @param x exponent in e^x.
 * @return constexpr double of e^x
 */
constexpr double
expc(double x)
{
    return exp2c(x * log2_e);
}

/**
 * @brief Constant evaluated natural log of x.
 *
 * @param x input number whereof the take the natural logarithm.
 * @return constexpr double of log(x)
 */
constexpr double
logc(double x)
{
    return log2c(x) / log2_e;
}

/**
 * @brief Performs the log_b(x). Done by way log rules, reducing the logarithm
 * to quotient of two log_2.
 *
 * @param x input number whereof to take the logarithm base b.
 * @param b base of the aforesaid logarithm.
 * @return constexpr double of log_b(x)
 */
constexpr double
logbc(double x, double b)
{
    return log2c(x) / log2c(b);
}

/**
 * @brief Constant evaluated power function, converting an expression:
 *
 * x^b = 2^(log_2(x)*b).
 *
 * @param x exponent base.
 * @param b exponent.
 * @return constexpr double of x^b.
 */
constexpr double
powc(double x, double b)
{
    return exp2c(log2c(x) * b);
}

/**
 * @brief Integer exponent overload for powc. If b is an integer, perform a
 * series of consecutive multiplications (much faster than the standard powc).
 *
 * @param x exponent base
 * @param b exponent
 * @return constexpr double of x^b.
 */
constexpr double
powc(double x, int b)
{
    bool inverse = false;
    if (b < 0) {
        inverse = true;
        b *= -1;
    }
    while (b-- > 0) {
        x *= x;
    }
    return inverse ? 1 / x : x;
}

/**
 * Performs a linear mapping 'twixt two intervals, [x1, y1] and [x2, y2].
 * Essentially a linear interpolation of the polynomial y = bx + a, satisfying:
 *
 *  a + bx1 = x2
 *  a + by1 = y2
 **/
template<typename T>
constexpr T
linear_map(T t, T x1, T y1, T x2, T y2)
{
    return (y2 - x2) / (y1 - x1) * t + x2;
}

constexpr double
gaussian(double x,
         double sigma,
         double a = 0,
         double b = 0,
         double c = 0,
         bool inverse = false)
{
    if (!(a && b && c)) {
        a = 1 / (sigma * sqrt2 * sqrt_pi);
        b = 0;
        c = sigma;
    }
    double n = inverse ? 1 : -1;
    auto exponent = n * powc((x - b) / (sqrt2 * c), 2);

    return a * powc(e, exponent);
}
