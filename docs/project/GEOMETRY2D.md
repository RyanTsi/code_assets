# 二维几何：公共数值约定与导航

本页只维护多个几何资产共同遵守的坐标域、精度、所有权与模板来源。每个算法的 API、退化输入、正确性与成本已迁入下方独立文档，不能仅凭领域公共约定推断特定算法适用。

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

几何算法的独立正文见 [资产文档目录](../README.md)。

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
