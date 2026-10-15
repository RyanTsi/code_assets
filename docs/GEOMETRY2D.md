# 二维几何资产

这批实现从用户提供的二维几何模板重新组织，覆盖原模板的点/向量、直线/线段、圆、多边形、凸包、最近点对、旋转卡壳和裁剪功能，并增加有类型的凸包、整数路径、凸多边形二分定位、显式组合配方和工作区复用。

## 调用方式

```cpp
#include <algorithm_assets/geometry/geometry2d.hpp>
#include <cstdint>
#include <vector>

namespace g = algorithm_assets::geometry;
std::vector<g::IntegerPoint> points{{0, 0}, {4, 0}, {4, 3}, {0, 3}};
auto hull = g::convex_hull<std::int64_t>(points);
auto diameter = g::convex_diameter(hull);   // 距离平方精确为 25
auto rectangle = g::minimum_area_rectangle(hull); // 浮点构造；面积 12
g::ClosestPairWorkspace<std::int64_t> workspace;
auto closest = workspace.solve(points);    // 工作区可再次使用
```

完整可运行示例是 `examples/geometry2d.cpp`。若只用某一模块，直接 include 对应头文件。几何总头文件是可选便利入口；原全库总头文件不自动引入几何，以控制解析成本。

<a id="kernel"></a>
## 数据表示与公共条件

- `Point2<T>` 与 `Vector2<T>` 区分位置和位移，均为两个连续坐标、16 字节；默认零初始化。`T` 只允许 binary64 `double` 和 `std::int64_t`。点减点得到向量，点加向量得到点，没有“点加点”这种语义混淆。
- `Point`/`Vector` 是浮点别名；`IntegerPoint` 是整数别名。没有输入输出流、万能头文件、全局可变 EPS 或强制继承 STL 容器。
- 浮点输入坐标必须有限且绝对值不超过 10¹⁰⁰；圆半径有限且在 [0,10¹⁰⁰]。构造出的点超过同一范围会抛 `overflow_error`。整数输入坐标在 ±10⁹，超界抛 `invalid_argument`，不会悄悄溢出或转为浮点。
- 浮点中间运算使用 `long double`，输出点仍是 binary64；不同平台的 `long double` 精度不同。这里**没有自适应精确浮点谓词，也没有精确有理交点构造**。零容差也不能把浮点变成精确几何；极小量可能下溢，接近退化情况的拓扑没有认证保证。距离平方若可检测到不同点被下溢成 0，会抛 `underflow_error`。不使用 fast-math/浮点重结合。
- 整数方向、点在线段上、线段相交、凸包、定位和距离平方比较使用 int64：差最多 2×10⁹，两个乘积的和/差最多 8×10¹⁸，小于 INT64_MAX。原始向量的 `dot`/`cross` 和算术运算是无检查的低层工具，调用者必须保证中间值可表示；上述高层算法会先校验坐标。
- `Tolerance(absolute, relative)` 默认绝对 10⁻¹²、相对 32×double epsilon。只对明确接收它的近似分类/构造生效。绝对容差具有对应量的单位（距离、角度/归一化叉积、面积）；单位或尺度变化时应显式配置。凸包、卡壳、最近点对、二分定位不使用 EPS 排序或 EPS 比较。点的 `==` 是精确坐标相等；近似比较用 `almost_equal`，**不应作为哈希/去重/排序等价关系**。
- 输入通常借用 `span<const Point2<T>>`，调用期间不得并发修改；返回值拥有其坐标，输入可随后销毁。`vertices()` 借用凸包存储，对象销毁、赋值或移动后不能继续依赖旧视图。结果复制是普通值复制，不是持久化数据结构。
- 纯函数可并发共享只读输入；`ClosestPairWorkspace` 的同一实例不能并发 solve/read，分别创建工作区即可。所有资产没有内建 checkpoint、rollback、undo、redo、版本分支或在线任意删除。输入分配或结果分配可能抛异常；失败的工作区下一次 solve 会重新赋值，可以复用，但保留的容量不保证不变。

<a id="api"></a>
## API、边界与成本

`n` 为输入点数，`h` 为已构造凸包的顶点数。以下为单位成本算术模型；整数符号判断与距离平方精确，浮点算法的几何正确性和理论界还依赖计算的谓词与所需几何关系一致。

| 头文件 / API | 输入输出与退化情况 | 时间 | 额外空间（输入排除） |
|---|---|---|---|
| `primitives.hpp`: `orientation`, `relative_position`, `squared_distance`, `distance`, `dot`, `cross`, `unit`, `rotate`, `polar` | 方向三态；位置六态（含退化）；距离与平方分开；零向量 unit 拒绝，旋转弧度必须有限 | O(1) | O(1)，无分配 |
| `linear.hpp`: `Line::through/from_direction/from_equation` | 直线是锚点+单位方向，不与线段共享继承；重合点、零方向、零法向量拒绝；默认直线是 x 轴；`Ax+By=C` 支持 C=0 | O(1) | 32 字节 Line |
| `projection`, `reflection`, `perpendicular_bisector`, `parallel`, `perpendicular`, `contains`, `intersects`, `intersection`, `closest_point`, `distance` | 有限线段允许两个端点相同；投影线段参数 clamp；直线交点 none/finite/infinite，线段交点 none/point/overlap（字典序区间端点）；整数线段只提供精确 membership/intersects，交点仍需浮点构造 | O(1) | O(1)，无分配 |
| `polygon.hpp`: `signed_area`, `area`, `perimeter`, `triangle_area` | 有序边界可 CW/CCW，不复制；空/点/线段面积 0；线段闭合周长为两倍长度；允许末尾重复首点；自交 signed_area 是代数面积 | O(n)；三角形 O(1) | O(1)，CompensatedSum |
| `is_convex` | **简单多边形是前提**，此 API 不验证自交；两种绕向均可；默认拒绝共线相邻边，可显式允许；不足三点、全共线或连续重复点 false | O(n) | O(1) |
| `locate(span,p[,tolerance])` | 奇偶填充 inside/boundary/outside；空 outside；点/线段只能 boundary/outside；浮点显式容差只用于边界距离，射线交叉采用半开规则 | O(n) | O(1) |
| `lowest_leftmost`, `sort_by_angle` | 空最低点 nullopt；极角从正 x 轴逆时针，原点零向量首先、同键按平方距离/精确字典序；原地改写输入顺序；角键预计算一次，严格弱序 | O(n) / O(n log n) | O(1) / n×sizeof(Key)（本机 32n，MSVC 可为 24n），排序栈 O(log n) |
| `convex_hull.hpp`: `convex_hull<T>`, `ConvexPolygon2<T>::from_points` | 任意点集→去重、去中间共线点的 CCW 边界，以字典序最小点开始，不重复首点；空/全重复/全共线→0/1/2 点；span 复制，vector 值参数可 move 转移输入 | O(n log n) | 输入副本最多 16n + 输出容量最多 16(n+1)；排序栈 O(log n)。move 省输入复制/新分配，不省这块同时存活的载荷 |
| `from_boundary` | 两种绕向及一次闭合点可接受；严格校验边界等于凸包的循环顺序；拒绝自交顺序、凹形、冗余共线及重复点。不是只检查局部拐向 | O(n log n) | 同凸包 |
| `locate(ConvexPolygon2,p)` | 凸包扇形二分定位，整数精确；0/1/2 点仍定义；无 EPS 二分，不复制、不缓存 | O(log(h+1)) | O(1) |
| `rotating_calipers.hpp`: `convex_diameter` | 只接受有类型的凸包；返回拥有两个坐标的 PointPair；空 nullopt，单点相同点对、平方 0；等高时检查两端点，避免遗漏平行对边 | O(h) | O(1) |
| `minimum_area_rectangle` | 只接受有类型凸包；空 nullopt；点四角相同、线段 `[a,b,b,a]`；其余四角 CCW，宽高/面积为浮点；等优解任取一个 | O(h) | O(1)，四角固定数组 |
| `convex_clip.hpp`: `clip_left` | 凸包+有向直线→**闭**左半平面的有序边界，保留线上的顶点；空输入/交为空→空；输出可能共线、未封装为严格凸包；整数输入也返回浮点构造 | O(h) | 输出最多 h+1 个点、16(h+1) 字节载荷，O(1) 工作空间 |
| `closest_pair.hpp`: `closest_pair<T>`, `ClosestPairWorkspace<T>::solve` | 任意点集；小于两点 nullopt；重复坐标→两个输入位置的相同坐标值、平方 0。返回值不含原始位置索引；等距对任取一对；`release()` 显式释放缓存 | O(n log n) | 两个点缓冲，正常新工作区 32n 字节；重复提前退出可只需 16n；递归栈 O(log n)。复用后以两向量 capacity 为准，不含分配器开销 |
| `point_set_bounds.hpp`: `point_set_diameter`, `minimum_bounding_rectangle` | 无序点集→凸包→卡壳的实际可调用配方；空/点/线段结果沿用组件定义 | O(n log n+h) | 凸包阶段输入副本和输出同时存在，卡壳 O(1)；不能重复加两阶段互不存活的空间 |

这些算法没有动态维护语义。读入全部点后求最近点对/凸包，不能当作在线插入、删点索引使用。

表中凸包输入副本的 16n 载荷针对 span 路径的新复制；转移已有 vector 时，其容量可能超过 n，按实际 capacity 计入峰值。容量字节数也不包含分配器开销。

目录把精确整数与近似浮点登记为不同资产，不能仅凭算法名互换。模板化矩形/裁剪等关联的 `condition` 注明实际选择的坐标内核；替代关系也注明转换与精度前提。当前 `condition` 是供人阅读的条件文本，不是已实现的通用组合规则。已有配方的坐标与结构条件通过查询的硬字段匹配，尚未搜索新组合。

<a id="circles"></a>
## 圆几何

`circle.hpp` 除圆与多边形的区域交集外，都是 O(1) 时间/空间、无堆分配。`IntersectionSet<T,N>` 的 `[0,count)` 是有效值，其他数组槽不具有输出意义；`kind` 区分 none、finite、infinite。

| API | 输入输出与特殊情况 |
|---|---|
| `locate(Circle,p)` | 圆盘内部、圆周、圆盘外；半径 0 的点圆允许 |
| `intersection(Circle,Line/Segment)` | **圆周**交点 0/1/2；线段端点和相切接触保留，零线段在圆周上为一个点；线段完全在圆盘内没有圆周交点 |
| `intersection(Circle,Circle)` | 圆周交点 0/1/2 或 infinite；同心不同半径 none；完全重合正半径 infinite；同一个点圆一个点；不把“近似重合”判为无限，无限分支要求坐标/半径精确相同 |
| `tangent_points` | 圆内点无切点，圆上点一个，圆外两个；点圆的唯一接触点是圆心。不是从圆内点开负数平方根 |
| `common_tangents`, `common_tangent_count` | 公切线固定容量最多 4；相切构造用切点+垂直法向量，避免两个相同端点；完全重合圆为 infinite、count API nullopt；两不同点圆一条直线；点圆与普通圆显式处理去重 |
| `circumcircle`, `incircle` | 退化/容差判为共线的三点 nullopt；有效结果 Circle；近退化构造超坐标/半径界仍可能抛异常 |
| `centers_through(a,b,r)` | 过两点的给定半径圆心 0/1/2 或 infinite；a=b,r>0 为无限，a=b,r=0 为唯一圆心 |
| `overlap_area` | **闭圆盘**公共面积；相离/外切 0，包含返回小圆面积；余弦 clamp；近外切使用 `theta-sin(theta)` 级数避免相消。此 API 不用容差提前吞掉薄透镜 |
| `disk_polygon_intersection` | 闭圆盘与**简单、非零面积**多边形区域；返回 disjoint/touching/overlapping（正面积交集）；点圆以及 0/1/2 点多边形不能有正面积交集；O(n)、O(1)；不替代圆周与边界交点集 |

## 正确性与优化依据

凸包的上下链始终保留左转，排序后每点在每条链至多入栈/出栈一次；整数坐标界保证这些转向精确。凸包二分定位将扇形定位到一个三角形，再检查最后一条边，射线两端单独处理。

最近点对的双缓冲在进入每个尚未处理的子区间时都保持 x 顺序。孩子交换源/输出角色，返回 y 有序的两半；父层只归并一次，不再整段拷贝回去。归并后源区间可以覆盖成条带，不破坏父层的输出。两侧各自最近距离保证条带候选的几何打包界，T(n)=2T(n/2)+O(n)；平方距离与平方横/纵界限避免内循环开方。算法保留点值，省去每点原始索引带来的额外 8 字节。显式工作区不存在 `inplace_merge` 的隐式分配；分配失败会抛异常，不会默默切换到较慢归并。

凸包直径检查所有反足点对；最小面积外接矩形有一条支撑边与凸包边平行，枚举各边足够。沿 CCW 边方向变化，三个支撑索引单调绕凸包前进；总推进 O(h)。严格改进才推进，等高单独检查，退化输入单独返回。另设 2h 推进预算检测数值不一致，避免无限循环；预算耗尽抛 `domain_error`，不自动退回 O(h²) 且声称线性。裁剪逐边保留左侧顶点并插入跨界交点，保留闭边界。

理论界不是无条件的平台最优声明。此批选择确定性分治和扫描，未加入期望线性最近点对、随机化空间哈希、任意精度坐标或自适应浮点谓词。实际实用输入、时空测量和负面结果见 [GEOMETRY_VALIDATION.md](GEOMETRY_VALIDATION.md)。

凸包构建会预留至多 n+1 个点以避免扫描扩容，但不会永久保留这份 n 规模容量：若输出容量超过 2h，先释放排序输入再紧凑复制结果。测试平台的最终载荷至多 32h；全重复/共线大输入只留下 1/2 个顶点的存储。紧凑步骤不增加原构建阶段的载荷峰值。

## 原模板迁移

| 原名称 | 新入口 / 行为变化 |
|---|---|
| Point/Vec/Points、Polygon 继承 vector | Point/Vector 强类型，普通 vector + span；ConvexPolygon2 显式强约束 |
| sgn/cmp/eq、Point 的 EPS `<`/`==` | Tolerance 显式近似比较；排序/去重/相等为精确键 |
| crs/dot/norm/abs/arg/normal/unit/rotate | cross/dot，平方长度 dot(v,v)，长度 hypot，角 atan2；perpendicular_ccw 正确表明 90° 方向，unit/rotate 自由函数 |
| proj/refl/midline/LPL/LOL/ccw | projection/reflection/perpendicular_bisector/parallel/perpendicular/orientation + relative_position |
| dis/intersect/crosspoint | distance/intersects/intersection/contains，交集带类型；旧点到线段距离递归错误已修正 |
| LTL/Area/CCWwhitO | lowest_leftmost/triangle_area/area/sort_by_angle；无空输入越界 |
| Iscontain/is_convex/len | locate/is_convex/perimeter；绕向独立；凸性仍要求简单多边形 |
| Convexhell/Diameter/RectangleArea/Convex_cut | convex_hull/convex_diameter/minimum_area_rectangle/clip_left；修正空/单点/共线死循环及未分配 resp 写越界；矩形四角按环序返回 |
| count_tangent/common_tangent/tangent_to_circle | common_tangent_count/common_tangents/tangent_points；无限情况明确表示，不产生退化切线 |
| circumcircle/incircle/center_given_radius/commonarea | 同名现代入口/centers_through/overlap_area；共线、相切、重合不依赖 assert、acos 不越界 |
| intersect(Circle,Polygon) | disk_polygon_intersection；旧实现只比较边界距离，不能说明圆盘与区域是否相交，现明确填充区域语义 |

输入原文按字节保存在 `provenance/geometry2d-template.cpp.txt`；SHA256 为 `f346ef1f9337127d605b213212e6b356cfc616ed5476a6046fe94cfbd9ef57de`。参考材料只阅读：`../oi-wiki-reference/docs/geometry/{2d,convex-hull,nearest-points,rotating-calipers}.md`。实现未复制参考 Wiki 代码，模板来源为用户提供，项目分发许可证仍沿用现有未选定状态。

关于浮点谓词为何不能只增加 EPS，参考 [Shewchuk 的原始说明](https://www.cs.cmu.edu/~quake/robust.html)。线性旋转卡壳的凸输入与复杂度可对照 [CGAL Bounding Volumes](https://doc.cgal.org/latest/Bounding_volumes/group__PkgBoundingVolumesRef.html)；这只是原理参考，不引入 CGAL。`merge` 与 `inplace_merge` 的成本区别见 [C++ 标准草案](https://eel.is/c++draft/alg.merge)。
