#include "math.hpp"

//double InversePersistence=NoiseFactor.a;    // for fBms. Persistence indicates how the frequencies are scaled when adding.  Normally a value of 2.0 is used for InversePersistance so that scale goes as 1/pow(invpersistence,i) or 1/pow(2,i). if ip=1->equal low and high noise.  if ip<1,more high noise.
//double Lacunarity=(INT)(NoiseFactor.b);     // for fBms. Lacunarity indicates how the frequency is changed for each iteration.  Normally a value of 2.0xxxx is used so frequency ~doubles at each iteration.  A value of 1 is the same as 1 octave since no harmonics.
//int n_octaves=(INT)(NoiseFactor.c);         // for fBms. number of octaves should be no more than log2(texturewidth) due to nyquist limit


///////////Perlin Noise setup

#define B 0x100
#define BM 0xff
#define NN 0x1000000     // This needs to be a very large value so that when setup adds a value to it it is always positive, regardless of size of argument which may get quite large in fracal sums

int pp[512];  // used by npnoise functions
int permutation[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
   };

int p[B + B + 2];   // used by pnoise functions
     
// gradients
double g1[B + B + 2];     // 1D
double g2[B + B + 2][2];  // 2D
double g3[B + B + 2][3];  // 3D 

#define setup(i,b0,b1,r0,r1) t = vec[i] + NN; b0 = ((int)t) & BM; b1 = (b0+1) & BM; r0 = t - (int)t; r1 = r0 - 1.;


static double g_precomputed[][3]= {
	{0.881927, 0.00933256, -0.471294},
	{0.783613, 0.523784, 0.334067},
	{0.16021, -0.224294, 0.961262},
	{0.476773, -0.630021, 0.612994},
	{-0.299992, -0.549216, 0.779979},
	{-0.437603, -0.389943, 0.810215},
	{0.474441, -0.809644, -0.345517},
	{0.250258, 0.896758, 0.36496},
	{0.664083, -0.723553, 0.188322},
	{-0.955856, -0.00391744, -0.293808},
	{0.430173, -0.337663, -0.837219},
	{-0.608608, 0.107774, 0.786118},
	{-0.161776, 0.0276203, 0.986441},
	{-0.900782, 0.407496, -0.15013},
	{0.67719, -0.735235, -0.0290224},
	{0.435259, 0.383347, 0.814613},
	{0.990155, 0.0594093, -0.12674},
	{-0.0900124, 0.565078, -0.820113},
	{0.877974, 0.321742, -0.354462},
	{-0.292866, -0.348273, -0.89047},
	{-0.493484, -0.626647, 0.603148},
	{-0.018251, -0.748291, 0.66312},
	{0.442048, -0.191684, 0.876271},
	{-0.039175, -0.415255, -0.908861},
	{-0.375172, -0.875401, 0.304827},
	{0.816216, -0.575061, 0.0556511},
	{0.688177, 0.478367, -0.545506},
	{-0.519943, -0.0310413, 0.853637},
	{0.732517, 0.677232, -0.0691053},
	{-0.387999, -0.383872, 0.837913},
	{-0.495871, 0.602129, -0.625742},
	{-0.557869, -0.825864, -0.0820395},
	{-0.252615, 0.959939, -0.121255},
	{0.656728, -0.682583, 0.320607},
	{-0.0722408, 0.995318, -0.0642141},
	{0.0264206, 0.970958, 0.237786},
	{0.566363, -0.257857, -0.782779},
	{-0.79241, 0.608663, -0.0401947},
	{-0.61328, -0.789435, -0.026097},
	{0.621471, -0.777896, 0.0930093},
	{0.179964, 0.439912, -0.879824},
	{0.920163, -0.387437, 0.0565012},
	{0.731388, 0.427997, 0.530933},
	{0.696311, -0.575795, 0.428499},
	{0.714037, 0.409693, -0.567718},
	{-0.954945, 0.296734, 0.00539517},
	{-0.261215, 0.931668, -0.252508},
	{-0.0466522, 0.419869, 0.906385},
	{0.901551, -0.353311, -0.249754},
	{-0.0734223, -0.682827, -0.726881},
	{0.789875, 0.490128, -0.368608},
	{-0.842201, -0.191409, 0.504044},
	{0.768506, 0.286146, 0.572292},
	{0.659914, -0.611391, -0.436708},
	{0.637383, -0.174766, 0.750467},
	{0.0181811, -0.645428, -0.763605},
	{0.903195, 0.428914, -0.0164967},
	{-0.680163, 0.216645, -0.700316},
	{0.157334, 0.875823, 0.456267},
	{-0.725857, 0.488843, -0.483905},
	{-0.268821, -0.604847, -0.749597},
	{-0.206278, 0.56349, -0.799955},
	{-0.759064, -0.586905, 0.281715},
	{-0.626585, 0.779282, -0.0105308},
	{0.453898, 0.841373, 0.293373},
	{0.335068, -0.687101, -0.644687},
	{0.605501, -0.70011, 0.378438},
	{0.368652, 0.741971, 0.559978},
	{0.200715, -0.0821105, 0.976203},
	{0.870031, -0.487923, 0.0705431},
	{0.657558, -0.307665, -0.687721},
	{-0.803072, 0.317494, -0.504255},
	{-0.940811, -0.338894, 0.00505812},
	{0.945164, -0.219413, -0.241917},
	{0.543321, -0.628883, 0.556155},
	{0.117745, -0.781828, 0.612275},
	{-0.162865, 0.35381, -0.921029},
	{0.625338, 0.695941, -0.353013},
	{0.823315, 0.476656, 0.308141},
	{-0.586069, -0.138442, 0.798346},
	{-0.991332, 0.097024, -0.0885871},
	{-0.781887, -0.414443, 0.465714},
	{-0.370439, -0.928134, -0.0366369},
	{0.371806, 0.87668, -0.305273},
	{0.0246669, -0.999011, -0.0370004},
	{-0.777502, -0.622795, -0.0872707},
	{0.881495, -0.25652, -0.39644},
	{0.32106, 0.840871, -0.435724},
	{-0.908547, -0.204628, -0.364237},
	{-0.18656, -0.457919, -0.869198},
	{-0.0928068, 0.625437, -0.774735},
	{-0.80303, -0.499758, -0.324629},
	{0.467011, -0.862955, -0.192896},
	{-0.844156, 0.427559, 0.32341},
	{-0.366754, -0.171152, 0.914439},
	{-0.37027, -0.590118, -0.717398},
	{-0.327903, -0.595403, 0.733468},
	{0.0786493, 0.992192, -0.0967992},
	{0.470555, 0.796323, 0.380063},
	{-0.778758, -0.0450149, -0.625707},
	{0.287529, 0.406964, 0.867011},
	{-0.935035, -0.21864, 0.279115},
	{-0.333575, -0.942711, 0.00483442},
	{-0.487224, -0.861555, -0.142602},
	{0.524416, 0.348022, 0.77709},
	{-0.315749, -0.874779, 0.367511},
	{0.718447, 0.662256, -0.212724},
	{-0.108332, 0.526184, -0.843442},
	{-0.312189, 0.70359, 0.638357},
	{0.719518, -0.575614, -0.388539},
	{-0.116052, 0.98644, -0.116052},
	{0.835012, -0.0392024, -0.548834},
	{-0.263718, -0.61403, 0.743922},
	{0.662808, -0.14685, 0.734248},
	{-0.567505, 0.823282, -0.0119895},
	{0.0315202, -0.737572, -0.674532},
	{-0.463101, 0.767773, 0.44279},
	{0.760856, -0.502826, -0.4102},
	{-0.884402, 0.136062, -0.446453},
	{-0.820505, -0.0444609, 0.569908},
	{0.261755, 0.251285, -0.931848},
	{0.538347, 0.507289, -0.672934},
	{-0.833848, -0.489191, -0.255713},
	{-0.981969, 0.0892699, -0.166637},
	{0.567306, 0.669131, -0.480029},
	{0.471825, 0.845723, -0.249266},
	{0.178178, -0.0633521, 0.981957},
	{0.531368, -0.315365, 0.786252},
	{0.568053, 0.0272665, -0.82254},
	{-0.660161, 0.746849, -0.0800196},
	{-0.743197, 0.276539, 0.609249},
	{-0.121776, -0.748052, 0.652371},
	{0.90717, 0.415575, -0.0658838},
	{0.899211, -0.333993, -0.282609},
	{-0.929721, 0.164693, -0.329387},
	{-0.301401, -0.943517, -0.137596},
	{0.572063, -0.631428, 0.523491},
	{0.960138, 0.262223, 0.0968206},
	{0.956128, -0.0670967, -0.285161},
	{0.492877, -0.341223, -0.800399},
	{-0.0509833, -0.846322, -0.530226},
	{-0.119676, 0.977353, 0.174527},
	{0.579728, -0.614119, 0.535512},
	{-0.0165382, 0.70701, 0.70701},
	{0.776577, -0.496146, -0.388288},
	{-0.267511, -0.312852, -0.911351},
	{0.043586, -0.966156, -0.254251},
	{-0.619005, -0.706807, 0.342428},
	{0.34909, 0.934329, -0.0718715},
	{-0.207273, 0.288556, -0.934759},
	{0.191337, 0.569106, 0.799692},
	{0.706407, -0.307333, -0.637601},
	{-0.549731, -0.768827, 0.326652},
	{-0.597983, -0.776328, 0.199328},
	{0, 0.21279, 0.977098},
	{0.836218, 0.380099, -0.395303},
	{-0.347158, -0.586415, -0.731846},
	{-0.74361, 0.358621, -0.5643},
	{-0.119613, 0.967308, -0.223625},
	{0.521332, 0.392343, -0.757813},
	{0.333037, -0.636249, 0.695898},
	{-0.736632, -0.0687523, -0.67279},
	{-0.368305, -0.830733, 0.417413},
	{0.802572, 0.401286, 0.441415},
	{0.618643, -0.520566, -0.588466},
	{0.340475, -0.89686, 0.282345},
	{0.416618, 0.901255, -0.119034},
	{0.980928, -0.159461, -0.11114},
	{-0.874596, -0.477977, -0.0813578},
	{0.617716, 0.677112, -0.399932},
	{-0.719814, 0.482602, 0.498962},
	{0.312856, -0.483006, 0.817818},
	{0.319034, -0.0294493, 0.947286},
	{-0.691378, -0.550129, 0.468353},
	{-0.435125, 0.255549, -0.863343},
	{-0.484711, -0.803235, 0.346222},
	{0.170271, 0.887471, -0.428256},
	{0.112697, -0.798272, 0.59166},
	{-0.790477, -0.560622, 0.246674},
	{0.145604, -0.208006, -0.967229},
	{0.125644, -0.225292, -0.966156},
	{0.685839, 0.719918, -0.106497},
	{-0.501538, -0.3869, -0.773801},
	{0.416413, 0.904898, -0.0880874},
	{-0.615626, -0.619649, -0.486867},
	{0.669855, 0.525021, 0.525021},
	{-0.270705, 0.0887557, 0.958562},
	{-0.184426, -0.493999, 0.849678},
	{0.207925, -0.855237, -0.474696},
	{-0.35043, -0.133075, 0.927087},
	{-0.890682, -0.356273, 0.282411},
	{0.39093, 0.349045, 0.85167},
	{0.346808, -0.579477, 0.737516},
	{0.677666, 0.687347, 0.261386},
	{0.448941, 0.104405, -0.887441},
	{-0.769922, -0.558906, 0.307969},
	{0.863871, 0.497192, -0.0807937},
	{0.88277, -0.387558, -0.265549},
	{0.316139, 0.724484, -0.612519},
	{-0.156561, -0.305093, 0.939365},
	{-0.863919, 0.493668, -0.0996829},
	{-0.399274, 0.432948, 0.808169},
	{-0.201097, -0.827772, -0.523788},
	{0.649832, -0.69096, -0.31669},
	{0.58329, -0.762109, -0.281001},
	{-0.0146116, 0.0535757, -0.998457},
	{0.0301203, 0.85843, 0.512046},
	{0.122289, -0.778574, -0.615522},
	{-0.6378, -0.621855, -0.454432},
	{0.572703, -0.381802, 0.725423},
	{0.283725, -0.671761, -0.684278},
	{0.482124, 0.583624, 0.653405},
	{-0.464375, -0.883445, 0.0622942},
	{-0.343074, 0.810902, 0.474066},
	{0.362148, 0.354905, -0.861912},
	{0.245597, -0.30927, 0.918713},
	{0.404371, 0.269581, 0.873963},
	{0.104848, 0.986809, 0.123351},
	{0.600225, -0.104626, -0.792958},
	{-0.27876, -0.931313, 0.234412},
	{0, 0.987007, -0.160676},
	{-0.570647, -0.723605, 0.388276},
	{0.865457, 0.397092, -0.305455},
	{0.619109, 0.754539, -0.217656},
	{-0.112209, -0.0290913, -0.993259},
	{-0.481007, -0.15838, -0.862292},
	{-0.805298, 0.592131, -0.0296065},
	{-0.101503, -0.882635, 0.45897},
	{-0.84889, 0.523013, 0.0764403},
	{-0.0694843, -0.67499, 0.734548},
	{-0.0984886, 0.977007, 0.189098},
	{0.823002, 0.564805, 0.0605149},
	{-0.996435, -0.08052, 0.0251625},
	{0.319592, -0.834754, -0.448382},
	{0.798493, 0.550685, 0.243219},
	{-0.265283, -0.631625, 0.728474},
	{-0.678481, 0.325353, 0.658642},
	{0.729404, 0.070763, -0.680414},
	{-0.95973, 0.280536, -0.0147651},
	{-0.866431, -0.404334, -0.292936},
	{-0.528207, 0.803314, 0.275108},
	{-0.459883, 0.27593, -0.84402},
	{-0.164752, 0.804749, -0.570295},
	{0.616383, 0.273302, 0.738497},
	{-0.122193, 0.882944, -0.453297},
	{-0.643681, 0.760714, 0.083595},
	{-0.0738983, 0.468023, 0.880621},
	{0.314462, -0.612935, 0.724862},
	{-0.35677, 0.932466, 0.0567588},
	{0.511392, -0.146711, -0.846731},
	{-0.185801, 0.170318, 0.967714},
	{-0.171952, -0.96137, -0.214941},
	{-0.81662, 0.361197, 0.450188},
	{-0.0538588, 0.65977, 0.749535},
	{0.317011, -0.926956, -0.20064},
	{0.190026, 0.740102, -0.645089},
	{0.881927, 0.00933256, -0.471294},
	{0.783613, 0.523784, 0.334067},
	{0.16021, -0.224294, 0.961262},
	{0.476773, -0.630021, 0.612994},
	{-0.299992, -0.549216, 0.779979},
	{-0.437603, -0.389943, 0.810215},
	{0.474441, -0.809644, -0.345517},
	{0.250258, 0.896758, 0.36496},
	{0.664083, -0.723553, 0.188322},
	{-0.955856, -0.00391744, -0.293808},
	{0.430173, -0.337663, -0.837219},
	{-0.608608, 0.107774, 0.786118},
	{-0.161776, 0.0276203, 0.986441},
	{-0.900782, 0.407496, -0.15013},
	{0.67719, -0.735235, -0.0290224},
	{0.435259, 0.383347, 0.814613},
	{0.990155, 0.0594093, -0.12674},
	{-0.0900124, 0.565078, -0.820113},
	{0.877974, 0.321742, -0.354462},
	{-0.292866, -0.348273, -0.89047},
	{-0.493484, -0.626647, 0.603148},
	{-0.018251, -0.748291, 0.66312},
	{0.442048, -0.191684, 0.876271},
	{-0.039175, -0.415255, -0.908861},
	{-0.375172, -0.875401, 0.304827},
	{0.816216, -0.575061, 0.0556511},
	{0.688177, 0.478367, -0.545506},
	{-0.519943, -0.0310413, 0.853637},
	{0.732517, 0.677232, -0.0691053},
	{-0.387999, -0.383872, 0.837913},
	{-0.495871, 0.602129, -0.625742},
	{-0.557869, -0.825864, -0.0820395},
	{-0.252615, 0.959939, -0.121255},
	{0.656728, -0.682583, 0.320607},
	{-0.0722408, 0.995318, -0.0642141},
	{0.0264206, 0.970958, 0.237786},
	{0.566363, -0.257857, -0.782779},
	{-0.79241, 0.608663, -0.0401947},
	{-0.61328, -0.789435, -0.026097},
	{0.621471, -0.777896, 0.0930093},
	{0.179964, 0.439912, -0.879824},
	{0.920163, -0.387437, 0.0565012},
	{0.731388, 0.427997, 0.530933},
	{0.696311, -0.575795, 0.428499},
	{0.714037, 0.409693, -0.567718},
	{-0.954945, 0.296734, 0.00539517},
	{-0.261215, 0.931668, -0.252508},
	{-0.0466522, 0.419869, 0.906385},
	{0.901551, -0.353311, -0.249754},
	{-0.0734223, -0.682827, -0.726881},
	{0.789875, 0.490128, -0.368608},
	{-0.842201, -0.191409, 0.504044},
	{0.768506, 0.286146, 0.572292},
	{0.659914, -0.611391, -0.436708},
	{0.637383, -0.174766, 0.750467},
	{0.0181811, -0.645428, -0.763605},
	{0.903195, 0.428914, -0.0164967},
	{-0.680163, 0.216645, -0.700316},
	{0.157334, 0.875823, 0.456267},
	{-0.725857, 0.488843, -0.483905},
	{-0.268821, -0.604847, -0.749597},
	{-0.206278, 0.56349, -0.799955},
	{-0.759064, -0.586905, 0.281715},
	{-0.626585, 0.779282, -0.0105308},
	{0.453898, 0.841373, 0.293373},
	{0.335068, -0.687101, -0.644687},
	{0.605501, -0.70011, 0.378438},
	{0.368652, 0.741971, 0.559978},
	{0.200715, -0.0821105, 0.976203},
	{0.870031, -0.487923, 0.0705431},
	{0.657558, -0.307665, -0.687721},
	{-0.803072, 0.317494, -0.504255},
	{-0.940811, -0.338894, 0.00505812},
	{0.945164, -0.219413, -0.241917},
	{0.543321, -0.628883, 0.556155},
	{0.117745, -0.781828, 0.612275},
	{-0.162865, 0.35381, -0.921029},
	{0.625338, 0.695941, -0.353013},
	{0.823315, 0.476656, 0.308141},
	{-0.586069, -0.138442, 0.798346},
	{-0.991332, 0.097024, -0.0885871},
	{-0.781887, -0.414443, 0.465714},
	{-0.370439, -0.928134, -0.0366369},
	{0.371806, 0.87668, -0.305273},
	{0.0246669, -0.999011, -0.0370004},
	{-0.777502, -0.622795, -0.0872707},
	{0.881495, -0.25652, -0.39644},
	{0.32106, 0.840871, -0.435724},
	{-0.908547, -0.204628, -0.364237},
	{-0.18656, -0.457919, -0.869198},
	{-0.0928068, 0.625437, -0.774735},
	{-0.80303, -0.499758, -0.324629},
	{0.467011, -0.862955, -0.192896},
	{-0.844156, 0.427559, 0.32341},
	{-0.366754, -0.171152, 0.914439},
	{-0.37027, -0.590118, -0.717398},
	{-0.327903, -0.595403, 0.733468},
	{0.0786493, 0.992192, -0.0967992},
	{0.470555, 0.796323, 0.380063},
	{-0.778758, -0.0450149, -0.625707},
	{0.287529, 0.406964, 0.867011},
	{-0.935035, -0.21864, 0.279115},
	{-0.333575, -0.942711, 0.00483442},
	{-0.487224, -0.861555, -0.142602},
	{0.524416, 0.348022, 0.77709},
	{-0.315749, -0.874779, 0.367511},
	{0.718447, 0.662256, -0.212724},
	{-0.108332, 0.526184, -0.843442},
	{-0.312189, 0.70359, 0.638357},
	{0.719518, -0.575614, -0.388539},
	{-0.116052, 0.98644, -0.116052},
	{0.835012, -0.0392024, -0.548834},
	{-0.263718, -0.61403, 0.743922},
	{0.662808, -0.14685, 0.734248},
	{-0.567505, 0.823282, -0.0119895},
	{0.0315202, -0.737572, -0.674532},
	{-0.463101, 0.767773, 0.44279},
	{0.760856, -0.502826, -0.4102},
	{-0.884402, 0.136062, -0.446453},
	{-0.820505, -0.0444609, 0.569908},
	{0.261755, 0.251285, -0.931848},
	{0.538347, 0.507289, -0.672934},
	{-0.833848, -0.489191, -0.255713},
	{-0.981969, 0.0892699, -0.166637},
	{0.567306, 0.669131, -0.480029},
	{0.471825, 0.845723, -0.249266},
	{0.178178, -0.0633521, 0.981957},
	{0.531368, -0.315365, 0.786252},
	{0.568053, 0.0272665, -0.82254},
	{-0.660161, 0.746849, -0.0800196},
	{-0.743197, 0.276539, 0.609249},
	{-0.121776, -0.748052, 0.652371},
	{0.90717, 0.415575, -0.0658838},
	{0.899211, -0.333993, -0.282609},
	{-0.929721, 0.164693, -0.329387},
	{-0.301401, -0.943517, -0.137596},
	{0.572063, -0.631428, 0.523491},
	{0.960138, 0.262223, 0.0968206},
	{0.956128, -0.0670967, -0.285161},
	{0.492877, -0.341223, -0.800399},
	{-0.0509833, -0.846322, -0.530226},
	{-0.119676, 0.977353, 0.174527},
	{0.579728, -0.614119, 0.535512},
	{-0.0165382, 0.70701, 0.70701},
	{0.776577, -0.496146, -0.388288},
	{-0.267511, -0.312852, -0.911351},
	{0.043586, -0.966156, -0.254251},
	{-0.619005, -0.706807, 0.342428},
	{0.34909, 0.934329, -0.0718715},
	{-0.207273, 0.288556, -0.934759},
	{0.191337, 0.569106, 0.799692},
	{0.706407, -0.307333, -0.637601},
	{-0.549731, -0.768827, 0.326652},
	{-0.597983, -0.776328, 0.199328},
	{0, 0.21279, 0.977098},
	{0.836218, 0.380099, -0.395303},
	{-0.347158, -0.586415, -0.731846},
	{-0.74361, 0.358621, -0.5643},
	{-0.119613, 0.967308, -0.223625},
	{0.521332, 0.392343, -0.757813},
	{0.333037, -0.636249, 0.695898},
	{-0.736632, -0.0687523, -0.67279},
	{-0.368305, -0.830733, 0.417413},
	{0.802572, 0.401286, 0.441415},
	{0.618643, -0.520566, -0.588466},
	{0.340475, -0.89686, 0.282345},
	{0.416618, 0.901255, -0.119034},
	{0.980928, -0.159461, -0.11114},
	{-0.874596, -0.477977, -0.0813578},
	{0.617716, 0.677112, -0.399932},
	{-0.719814, 0.482602, 0.498962},
	{0.312856, -0.483006, 0.817818},
	{0.319034, -0.0294493, 0.947286},
	{-0.691378, -0.550129, 0.468353},
	{-0.435125, 0.255549, -0.863343},
	{-0.484711, -0.803235, 0.346222},
	{0.170271, 0.887471, -0.428256},
	{0.112697, -0.798272, 0.59166},
	{-0.790477, -0.560622, 0.246674},
	{0.145604, -0.208006, -0.967229},
	{0.125644, -0.225292, -0.966156},
	{0.685839, 0.719918, -0.106497},
	{-0.501538, -0.3869, -0.773801},
	{0.416413, 0.904898, -0.0880874},
	{-0.615626, -0.619649, -0.486867},
	{0.669855, 0.525021, 0.525021},
	{-0.270705, 0.0887557, 0.958562},
	{-0.184426, -0.493999, 0.849678},
	{0.207925, -0.855237, -0.474696},
	{-0.35043, -0.133075, 0.927087},
	{-0.890682, -0.356273, 0.282411},
	{0.39093, 0.349045, 0.85167},
	{0.346808, -0.579477, 0.737516},
	{0.677666, 0.687347, 0.261386},
	{0.448941, 0.104405, -0.887441},
	{-0.769922, -0.558906, 0.307969},
	{0.863871, 0.497192, -0.0807937},
	{0.88277, -0.387558, -0.265549},
	{0.316139, 0.724484, -0.612519},
	{-0.156561, -0.305093, 0.939365},
	{-0.863919, 0.493668, -0.0996829},
	{-0.399274, 0.432948, 0.808169},
	{-0.201097, -0.827772, -0.523788},
	{0.649832, -0.69096, -0.31669},
	{0.58329, -0.762109, -0.281001},
	{-0.0146116, 0.0535757, -0.998457},
	{0.0301203, 0.85843, 0.512046},
	{0.122289, -0.778574, -0.615522},
	{-0.6378, -0.621855, -0.454432},
	{0.572703, -0.381802, 0.725423},
	{0.283725, -0.671761, -0.684278},
	{0.482124, 0.583624, 0.653405},
	{-0.464375, -0.883445, 0.0622942},
	{-0.343074, 0.810902, 0.474066},
	{0.362148, 0.354905, -0.861912},
	{0.245597, -0.30927, 0.918713},
	{0.404371, 0.269581, 0.873963},
	{0.104848, 0.986809, 0.123351},
	{0.600225, -0.104626, -0.792958},
	{-0.27876, -0.931313, 0.234412},
	{0, 0.987007, -0.160676},
	{-0.570647, -0.723605, 0.388276},
	{0.865457, 0.397092, -0.305455},
	{0.619109, 0.754539, -0.217656},
	{-0.112209, -0.0290913, -0.993259},
	{-0.481007, -0.15838, -0.862292},
	{-0.805298, 0.592131, -0.0296065},
	{-0.101503, -0.882635, 0.45897},
	{-0.84889, 0.523013, 0.0764403},
	{-0.0694843, -0.67499, 0.734548},
	{-0.0984886, 0.977007, 0.189098},
	{0.823002, 0.564805, 0.0605149},
	{-0.996435, -0.08052, 0.0251625},
	{0.319592, -0.834754, -0.448382},
	{0.798493, 0.550685, 0.243219},
	{-0.265283, -0.631625, 0.728474},
	{-0.678481, 0.325353, 0.658642},
	{0.729404, 0.070763, -0.680414},
	{-0.95973, 0.280536, -0.0147651},
	{-0.866431, -0.404334, -0.292936},
	{-0.528207, 0.803314, 0.275108},
	{-0.459883, 0.27593, -0.84402},
	{-0.164752, 0.804749, -0.570295},
	{0.616383, 0.273302, 0.738497},
	{-0.122193, 0.882944, -0.453297},
	{-0.643681, 0.760714, 0.083595},
	{-0.0738983, 0.468023, 0.880621},
	{0.314462, -0.612935, 0.724862},
	{-0.35677, 0.932466, 0.0567588},
	{0.511392, -0.146711, -0.846731},
	{-0.185801, 0.170318, 0.967714},
	{-0.171952, -0.96137, -0.214941},
	{-0.81662, 0.361197, 0.450188},
	{-0.0538588, 0.65977, 0.749535},
	{0.317011, -0.926956, -0.20064},
	{0.190026, 0.740102, -0.645089},
	{0.881927, 0.00933256, -0.471294},
	{0.783613, 0.523784, 0.334067},
};

//#define fade(t) ( t )                                    // linear linterpolation   fast but poor
//#define fade(t) ( t * t * (3. - 2. * t) )                // cubic interplation
#define fade(t) ( t * t * t * (t * (t * 6 - 15) + 10) )  // quintic interpolation does better on integer lattice points but mostly only if derivatives of noise are used such as in bumpmapping

#define lerp(t, a, b) ( a + t * (b - a) )      // lerp="linear interpolation"

static void normalize2(double v[2])
{
        double s;

	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}
/* to avoid unused warning
static void normalize3(double v[3])
{
        double s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}
*/
//double pnoise1(double vec[]);

/* Coherent Perlin noise function over 1, 2 or 3 dimensions */
// actually lattice noise of the gradient variety

static double pnoise1(double vec[])
{  
    int bx0=0, bx1=0;
    double rx0=0., rx1=0., sx=0., t=0., u=0., v=0.;

    //  #define setup(i,b0,b1,r0,r1) t = vec[i] + NN; b0 = ((int)t) & BM; b1 = (b0+1) & BM; r0 = t - (int)t; r1 = r0 - 1.;

	setup(0, bx0,bx1, rx0,rx1);  // by define this is dependent on vec[0]
       
    sx = fade(rx0);
   
	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];
       
    return 2.1*lerp(sx, u, v);      // essentially scales a -.5 to +.5 distribution to -1 to 1
}

static double pnoise2(double vec[])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
        double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	int  i, j;

    setup(0, bx0,bx1, rx0,rx1);
    setup(1, by0,by1, ry0,ry1);

    i = p[ bx0 ];
    j = p[ bx1 ];

    b00 = p[ i + by0 ];
    b10 = p[ j + by0 ];
    b01 = p[ i + by1 ];
    b11 = p[ j + by1 ];

    sx = fade(rx0);
    sy = fade(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

    return 1.5*lerp(sy, a, b);       // essentially scales a gaussian -.7 to +.7 distribution to -1 to 1
}

int test_count=0;

static double pnoise3(double vec[])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
        double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

        i = p[ bx0 ];
        j = p[ bx1 ];

        b00 = p[ i + by0 ];
        b10 = p[ j + by0 ];
        b01 = p[ i + by1 ];
        b11 = p[ j + by1 ];

        t  = fade(rx0);
        sy = fade(ry0);
        sz = fade(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )


        q = g3[ (b00 + bz0)     ] ; u = at3(rx0,ry0,rz0);
        q = g3[ (b10 + bz0)     ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

        q = g3[ (b01 + bz0)     ] ; u = at3(rx0,ry1,rz0);
        q = g3[ (b11 + bz0)     ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

        q = g3[ (b00 + bz1)     ] ; u = at3(rx0,ry0,rz1);
        q = g3[ (b10 + bz1)     ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

        q = g3[ (b01 + bz1)     ] ; u = at3(rx0,ry1,rz1);
        q = g3[ (b11 + bz1)     ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

        d = lerp(sy, a, b);

        return 1.5*lerp(sz, c, d);       // essentially scales a gaussian -.7 to +.7 distribution to -1 to 1
}


/* --- fBm harmonic summing functions - PDB --------------------------*/
// actually these are fBM (fractional Brownian motion) functions

// -1 to +1 noise
double fBm1DNoise(double x,double InversePersistence,double Lacunarity,int n_octaves)
{
  
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   if (InversePersistence<.5) InversePersistence=.5;
   if (InversePersistence>4.0) InversePersistence=4.0;
   if (Lacunarity<1.0) Lacunarity=1.0;
   if (Lacunarity>4.0) Lacunarity=4.0;

   int i;
   double val,sum = 0;
   double p[1],scale = 1;

   p[0] = x;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise1(p);
      sum += val / scale;
      scale *= InversePersistence;
      p[0] *= Lacunarity;
   }
   return(sum);
}

double FastfBm1DNoise(double x,int n_octaves)  // fixed invpersistence=2. and lacunarity=2.
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   
   int i;
   double val,sum = 0;
   double p[1],scale = 1;

   p[0] = x;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise1(p);
      sum += val / scale;
      scale *= 2.;
      p[0] *= 2.02345;
   }
   return(sum);
}

// -1 to +1 noise
double fBm2DNoise(double x,double y,double InversePersistence,double Lacunarity,int n_octaves,bool *NoiseResetFlag)
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   if (InversePersistence<.5) InversePersistence=.5;
   if (InversePersistence>4.0) InversePersistence=4.0;
   if (Lacunarity<1.0) Lacunarity=1.0;
   if (Lacunarity>4.0) Lacunarity=4.0;

   int i;
   double val,sum = 0;
   double p[2],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static double   currentInversePersistence=0;
   static double   currentLacunarity=0;
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if ((InversePersistence!=currentInversePersistence)||(Lacunarity!=currentLacunarity)||(n_octaves!=currentnoctaves))
   {
           double dfscale=1.;                  // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= InversePersistence;
           }
           normalscale=sqrt(normalscale);

           currentInversePersistence=InversePersistence;
           currentLacunarity=Lacunarity;
           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }


   p[0] = x;
   p[1] = y;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise2(p);
      sum += val / scale;
      scale *= InversePersistence;
      p[0] *= Lacunarity;
      p[1] *= Lacunarity;
   }
   return(sum/(1.2*normalscale));  // the 1.2 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}

double FastfBm2DNoise(double x,double y,int n_octaves,bool *NoiseResetFlag) // fixed invpersistence=2. and lacunarity=2.
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   
   int i;
   double val,sum = 0;
   double p[2],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if (n_octaves!=currentnoctaves)
   {
           double dfscale=1.;                    // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= 2.;
           }
           normalscale=sqrt(normalscale);

           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }


   p[0] = x;
   p[1] = y;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise2(p);
      sum += val / scale;
      scale *= 2.;
      p[0] *= 2.05645;  // to avoid artifacts avoid lacunarity of 2.0
      p[1] *= 2.05467;  // to avoid artifacts avoid lacunarity of 2.0
   }
   return(sum/(1.2*normalscale));  // the 1.2 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}

double grad3(int hash, double x, double y, double z)
  {
    int     h = hash & 15;       // CONVERT LO 4 BITS OF HASH CODE
    double  u = h < 8 ? x : y,   // INTO 12 GRADIENT DIRECTIONS.
            v = h < 4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
  }


double npnoise3(double vec[])      // new improved perlin noise3
  {
    double x=vec[0];
    double y=vec[1];
    double z=vec[2];

    int   X = (int)floor(x) & 255,             // FIND UNIT CUBE THAT
          Y = (int)floor(y) & 255,             // CONTAINS POINT.
          Z = (int)floor(z) & 255;

    x -= floor(x);                             // FIND RELATIVE X,Y,Z
    y -= floor(y);                             // OF POINT IN CUBE.
    z -= floor(z);

    double  u = fade(x),                       // COMPUTE FADE CURVES
            v = fade(y),                       // FOR EACH OF X,Y,Z.
            w = fade(z);

    int  A = p[X]+Y,
         AA = p[A]+Z,
         AB = p[A+1]+Z, // HASH COORDINATES OF
         C = p[X+1]+Y,
         BA = p[C]+Z,
         BB = p[C+1]+Z; // THE 8 CUBE CORNERS,

    return lerp(w,lerp(v,lerp(u, grad3(p[AA  ], x, y, z),   // AND ADD
                         grad3(p[BA  ], x-1, y, z)),        // BLENDED
                 lerp(u, grad3(p[AB  ], x, y-1, z),         // RESULTS
                         grad3(p[BB  ], x-1, y-1, z))),     // FROM  8
                 lerp(v, lerp(u, grad3(p[AA+1], x, y, z-1 ),// CORNERS
                         grad3(p[BA+1], x-1, y, z-1)),      // OF CUBE
                 lerp(u, grad3(p[AB+1], x, y-1, z-1),
                         grad3(p[BB+1], x-1, y-1, z-1))));
  }


static double grad4(int hash, double x, double y, double z, double w)
{
   int h = hash & 31; // CONVERT LO 5 BITS OF HASH TO 32 GRAD DIRECTIONS.
   double a=y,b=z,c=w;            // X,Y,Z
   switch (h >> 3)
   {          // OR, DEPENDING ON HIGH ORDER 2 BITS:
   case 1: a=w;b=x;c=y;break;     // W,X,Y
   case 2: a=z;b=w;c=x;break;     // Z,W,X
   case 3: a=y;b=z;c=w;break;     // Y,Z,W
   };
   return ((h&4)==0 ? -a:a) + ((h&2)==0 ? -b:b) + ((h&1)==0 ? -c:c);
}

double npnoise4(double vec[])
{
      double x=vec[0];
      double y=vec[1];
      double z=vec[2];
      double w=vec[3];

      int X = (int)floor(x) & 255;                  // FIND UNIT HYPERCUBE
      int Y = (int)floor(y) & 255;                  // THAT CONTAINS POINT.
      int Z = (int)floor(z) & 255;
      int W = (int)floor(w) & 255;

      x -= floor(x);                                // FIND RELATIVE X,Y,Z,W
      y -= floor(y);                                // OF POINT IN CUBE.
      z -= floor(z);
      w -= floor(w);

      double a = fade(x);                                // COMPUTE FADE CURVES
      double b = fade(y);                                // FOR EACH OF X,Y,Z,W.
      double c = fade(z);
      double d = fade(w);

      int A   = pp[X]+Y;      // HASH COORDINATES OF
      int AA  = pp[A]+Z;        // THE 16 CORNERS OF
      int AB  = pp[A+1]+Z;      // THE HYPERCUBE.
      int C   = pp[X+1]+Y;
      int BA  = pp[C]+Z;
      int BB  = pp[C+1]+Z;
      int AAA = pp[AA]+W;
      int AAB = pp[AA+1]+W;
      int ABA = pp[AB]+W;
      int ABB = pp[AB+1]+W;
      int BAA = pp[BA]+W;
      int BAB = pp[BA+1]+W;
      int BBA = pp[BB]+W;
      int BBB = pp[BB+1]+W;

      return lerp(d,                                     // INTERPOLATE DOWN.
          lerp(c,lerp(b,lerp(a,grad4(pp[AAA  ], x  , y  , z  , w),
                               grad4(pp[BAA  ], x-1, y  , z  , w)),
                        lerp(a,grad4(pp[ABA  ], x  , y-1, z  , w),
                               grad4(pp[BBA  ], x-1, y-1, z  , w))),

                 lerp(b,lerp(a,grad4(pp[AAB  ], x  , y  , z-1, w),
                               grad4(pp[BAB  ], x-1, y  , z-1, w)),
                        lerp(a,grad4(pp[ABB  ], x  , y-1, z-1, w),
                               grad4(pp[BBB  ], x-1, y-1, z-1, w)))),

          lerp(c,lerp(b,lerp(a,grad4(pp[AAA+1], x  , y  , z  , w-1),
                               grad4(pp[BAA+1], x-1, y  , z  , w-1)),
                        lerp(a,grad4(pp[ABA+1], x  , y-1, z  , w-1),
                               grad4(pp[BBA+1], x-1, y-1, z  , w-1))),

                 lerp(b,lerp(a,grad4(pp[AAB+1], x  , y  , z-1, w-1),
                               grad4(pp[BAB+1], x-1, y  , z-1, w-1)),
                        lerp(a,grad4(pp[ABB+1], x  , y-1, z-1, w-1),
                               grad4(pp[BBB+1], x-1, y-1, z-1, w-1)))));
}


// -1 to +1 noise
double fBm3DNoise(double x,double y,double z,double InversePersistence,double Lacunarity,int n_octaves, bool *NoiseResetFlag)
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   if (InversePersistence<.5) InversePersistence=.5;
   if (InversePersistence>4.0) InversePersistence=4.0;
   if (Lacunarity<1.0) Lacunarity=1.0;
   if (Lacunarity>4.0) Lacunarity=4.0;

   int i;
   double val,sum = 0;
   double p[3],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static double   currentInversePersistence=0;
   static double   currentLacunarity=0;
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if ((InversePersistence!=currentInversePersistence)||(Lacunarity!=currentLacunarity)||(n_octaves!=currentnoctaves))
   {
           double dfscale=1.;                  // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= InversePersistence;
           }
           normalscale=sqrt(normalscale);

           currentInversePersistence=InversePersistence;
           currentLacunarity=Lacunarity;
           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }

   p[0] = x;
   p[1] = y;
   p[2] = z;
   val=0; sum=0;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise3(p);
      sum += val / scale;
      scale *= InversePersistence;
      p[0] *= Lacunarity;
      p[1] *= Lacunarity;
      p[2] *= Lacunarity;
   }
   return(sum/(1.1*normalscale));  // the 1.1 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}

double FastfBm3DNoise(double x,double y,double z,int n_octaves,bool *NoiseResetFlag) // fixed invpersistence=2. and lacunarity=2.
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   
   int i;
   double val,sum = 0;
   double p[3],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if (n_octaves!=currentnoctaves)
   {
           double dfscale=1.;                    // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= 2.;
           }
           normalscale=sqrt(normalscale);

           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }


   p[0] = x;
   p[1] = y;
   p[2] = z;
   for (i=0;i<n_octaves;i++)
   {
      val = pnoise3(p);
      sum += val / scale;
      scale *= 2.;
      p[0] *= 2.05656;  // to avoid artifacts avoid lacunarity of 2.0
      p[1] *= 2.06756;  // to avoid artifacts avoid lacunarity of 2.0
      p[2] *= 2.06345;  // to avoid artifacts avoid lacunarity of 2.0
   }
   return(sum/(1.1*normalscale));  // the 1.1 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}

// -1 to +1 noise
double fBm4DNoise(double x,double y,double z,double w,double InversePersistence,double Lacunarity,int n_octaves, bool *NoiseResetFlag)
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
   if (InversePersistence<.5) InversePersistence=.5;
   if (InversePersistence>4.0) InversePersistence=4.0;
   if (Lacunarity<1.0) Lacunarity=1.0;
   if (Lacunarity>4.0) Lacunarity=4.0;

   int i;
   double val,sum = 0;
   double p[4],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static double   currentInversePersistence=0;
   static double   currentLacunarity=0;
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if ((InversePersistence!=currentInversePersistence)||(Lacunarity!=currentLacunarity)||(n_octaves!=currentnoctaves))
   {
           double dfscale=1.;                  // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= InversePersistence;
           }
           normalscale=sqrt(normalscale);

           currentInversePersistence=InversePersistence;
           currentLacunarity=Lacunarity;
           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }


   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = w;
   for (i=0;i<n_octaves;i++)
   {
      val = npnoise4(p);
      sum += val / scale;
      scale *= InversePersistence;
      p[0] *= Lacunarity;
      p[1] *= Lacunarity;
      p[2] *= Lacunarity;
      p[3] *= Lacunarity;
   }
   return(sum/(1.3*normalscale));  // the 1.3 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}

double FastfBm4DNoise(double x,double y,double z,double w,int n_octaves, bool *NoiseResetFlag)
{
   if (n_octaves<1) n_octaves=1;
   if (n_octaves>6) n_octaves=6;
  
   int i;
   double val,sum = 0;
   double p[4],scale = 1;

   static double   normalscale=1.;   // used to renormalize added octaves
   static int      currentnoctaves=0;

   if (*NoiseResetFlag)
   if (n_octaves!=currentnoctaves)
   {
           double dfscale=1.;                    // based on sqrt of sum of squares of scales
           double inversescale=1.;
           normalscale=0;
           for (i=0;i<n_octaves;i++)
           {
              inversescale = 1./dfscale;
              normalscale += inversescale*inversescale;
              dfscale *= 2.;
           }
           normalscale=sqrt(normalscale);

           currentnoctaves=n_octaves;
           *NoiseResetFlag=false;
   }


   p[0] = x;
   p[1] = y;
   p[2] = z;
   p[3] = w;
   for (i=0;i<n_octaves;i++)
   {
      val = npnoise4(p);
      sum += val / scale;
      scale *= 2.;
      p[0] *= 2.05654;   // to avoid artifacts avoid lacunarity of 2.0
      p[1] *= 2.02384;
      p[2] *= 2.02378;
      p[3] *= 2.04532;
   }
   return(sum/(1.3*normalscale));  // the 1.3 is fudge to make 8 octave sum correct, but introduces some error at low octave counts
}



void initPerlin(void)
{
        int i, j;

        for (i=0; i< 256; ++i)
          pp[256+i]=pp[i]=permutation[i];

        for (i = 0 ; i < B ; i++)
        {
            p[i] = pp[i];

            g1[i] = g_precomputed[i][0];

		    for (j = 0 ; j < 2 ; j++)
                g2[i][j] = g_precomputed[i][j];
		    normalize2(g2[i]);

            for (j = 0 ; j < 3 ; j++)
                            g3[i][j] = g_precomputed[i][j];
                // normalize3(g3[i]);  // already normalized
	    }

        for (i = 0 ; i < B + 2 ; i++)
        {
            p[B + i] = p[i];
            g1[B + i] = g1[i];
            for (j = 0 ; j < 2 ; j++)
                g2[B + i][j] = g2[i][j];
            for (j = 0 ; j < 3 ; j++)
                g3[B + i][j] = g3[i][j];
	    }

}



