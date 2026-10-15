(() => {
  'use strict';
  const data = JSON.parse(document.getElementById('catalog-data').textContent);
  const $ = id => document.getElementById(id);
  const NS = 'http://www.w3.org/2000/svg';
  const kinds = {algorithm: '算法', representation: '数据表示', recipe: '组合配方'};
  const types = {alternative: '可替代', uses: '依赖使用', composes_into: '参与组合', representation_for: '提供表示'};
  const colors = {alternative: '#b39aeb', uses: '#86aacd', composes_into: '#d3b680', representation_for: '#8bbfac'};
  const statuses = {supported: '支持', unsupported: '不支持', conditional: '有条件', unknown: '未确定'};
  const capabilities = {persistence: '持久化', checkpoint: '检查点', rollback: '回滚', undo: '撤销', redo: '重做', arbitrary_deletion: '任意删除', concurrent_reads: '并发读取', concurrent_writes: '并发写入'};
  const fieldNames = {max_n: '最大规模 n', errors: '错误行为', overflow: '溢出处理', threading: '并发约束', ownership: '所有权', space: '额外空间', guarantee: '复杂度保证', search: '搜索', path: '路径恢复', precision: '精度', build: '构建', query: '查询', update: '更新', initialization: '初始化'};
  const selectedKinds = new Set(Object.keys(kinds)), selectedTypes = new Set(Object.keys(types));
  const nodeById = new Map(data.nodes.map(node => [node.id, node]));
  const state = {selected: null, scale: 1, tx: 0, ty: 0, paused: matchMedia('(prefers-reduced-motion: reduce)').matches, ticks: 0, initialFit: true};
  let width = 0, height = 0, frame = null, drag = null, visibleNodes = [], visibleEdges = [];
  const escape = value => String(value).replace(/[&<>"']/g, ch => ({'&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'}[ch]));
  const displayValue = value => typeof value === 'object' ? JSON.stringify(value) : String(value);
  const label = id => nodeById.get(id).label;
  function svg(tag, attrs, parent) {
    const element = document.createElementNS(NS, tag);
    Object.entries(attrs).forEach(([key, value]) => element.setAttribute(key, value));
    parent.append(element);
    return element;
  }
  function filters(container, options, set, relation) {
    Object.entries(options).forEach(([key, name]) => {
      const item = document.createElement('label'), input = document.createElement('input');
      input.type = 'checkbox'; input.checked = true; input.dataset.filter = key;
      item.append(input, document.createTextNode(name));
      if (relation) { const line = document.createElement('span'); line.className = `relation-key ${key}`; item.append(line); }
      input.addEventListener('change', () => { input.checked ? set.add(key) : set.delete(key); refresh(); });
      container.append(item);
    });
  }
  filters($('kind-filters'), kinds, selectedKinds);
  filters($('relation-filters'), types, selectedTypes, true);
  $('total-assets').textContent = data.nodes.length;
  $('total-edges').textContent = data.edges.length;

  data.nodes.forEach((node, index) => {
    const angle = index / data.nodes.length * Math.PI * 2 - Math.PI / 2;
    node.x = Math.cos(angle) * 190; node.y = Math.sin(angle) * 160; node.vx = node.vy = 0;
    const group = svg('g', {class: 'node', 'data-id': node.id}, $('nodes'));
    svg('circle', {class: 'halo', r: 23}, group);
    const fill = node.kind === 'representation' ? '#8bbfac' : node.kind === 'recipe' ? '#d3b680' : '#b39aeb';
    if (node.kind === 'representation') svg('rect', {class: 'shape', x: -7, y: -7, width: 14, height: 14, rx: 2, fill}, group);
    else if (node.kind === 'recipe') svg('path', {class: 'shape', d: 'M0,-10 L10,0 L0,10 L-10,0 Z', fill}, group);
    else svg('circle', {class: 'shape', r: 7, fill}, group);
    svg('circle', {r: 22, fill: 'transparent'}, group);
    svg('text', {y: 33}, group).textContent = node.label;
    node.element = group;
    group.addEventListener('pointerdown', event => {
      if (event.button !== 0) return;
      event.stopPropagation();
      $('viewport').setPointerCapture(event.pointerId);
      drag = {node, startX: event.clientX, startY: event.clientY, moved: false, pointer: event.pointerId};
    });
  });
  data.edges.forEach(edge => {
    const group = svg('g', {class: 'edge-group'}, $('edges'));
    edge.line = svg('line', {class: 'edge-line'}, group);
    edge.hit = svg('line', {class: 'edge hit'}, group);
    edge.text = svg('text', {class: 'edge-label'}, group);
    edge.element = group;
    edge.hit.addEventListener('pointerenter', event => {
      const relations = activeRelations(edge);
      $('tooltip').textContent = relations.map(r => `${label(r.source)} ${r.type === 'alternative' ? '↔' : '→'} ${label(r.target)} · ${types[r.type]}${r.condition ? `\n条件：${r.condition}` : ''}`).join('\n');
      $('tooltip').style.whiteSpace = 'pre-line';
      const box = $('viewport').getBoundingClientRect();
      $('tooltip').style.left = `${Math.max(8, Math.min(event.clientX - box.left + 12, width - 295))}px`;
      $('tooltip').style.top = `${Math.max(8, Math.min(event.clientY - box.top + 12, height - 100))}px`;
      $('tooltip').hidden = false;
    });
    edge.hit.addEventListener('pointerleave', () => { $('tooltip').hidden = true; });
  });

  function activeRelations(edge) { return edge.relations.filter(relation => selectedTypes.has(relation.type)); }
  function refresh() {
    const query = $('search').value.trim().toLowerCase();
    let nodes = data.nodes.filter(node => selectedKinds.has(node.kind) &&
      [node.label, node.name, node.id, node.problem].some(value => value.toLowerCase().includes(query)));
    const ids = new Set(nodes.map(node => node.id));
    if (state.selected && !ids.has(state.selected)) select(null, false);
    let edges = data.edges.filter(edge => activeRelations(edge).length && ids.has(edge.source) && ids.has(edge.target));
    if ($('neighbors').checked && state.selected) {
      const keep = new Set([state.selected]);
      edges.forEach(edge => { if (edge.source === state.selected || edge.target === state.selected) { keep.add(edge.source); keep.add(edge.target); } });
      nodes = nodes.filter(node => keep.has(node.id));
      edges = edges.filter(edge => keep.has(edge.source) && keep.has(edge.target));
    }
    visibleNodes = nodes; visibleEdges = edges;
    const shown = new Set(nodes.map(node => node.id));
    data.nodes.forEach(node => { node.element.style.display = shown.has(node.id) ? '' : 'none'; });
    data.edges.forEach(edge => { edge.element.style.display = edges.includes(edge) ? '' : 'none'; });
    $('asset-list').replaceChildren();
    nodes.forEach(node => {
      const button = document.createElement('button');
      button.className = `asset-item${state.selected === node.id ? ' active' : ''}`;
      button.setAttribute('aria-pressed', state.selected === node.id);
      button.innerHTML = `<span class="kind-dot ${escape(node.kind)}"></span>${escape(node.label)}<small>${node.kind === 'algorithm' ? '' : escape(kinds[node.kind])}</small>`;
      button.addEventListener('click', () => select(node.id));
      $('asset-list').append(button);
    });
    $('asset-count').textContent = `${nodes.length} / ${data.nodes.length}`;
    $('graph-count').textContent = `${nodes.length} 个节点 · ${edges.length} 条连线`;
    $('empty').hidden = nodes.length > 0;
    $('tooltip').hidden = true;
    emphasize(); updatePositions(); startLayout();
  }
  function emphasize() {
    const neighbors = new Set([state.selected]);
    visibleEdges.forEach(edge => { if (edge.source === state.selected || edge.target === state.selected) { neighbors.add(edge.source); neighbors.add(edge.target); } });
    data.nodes.forEach(node => {
      node.element.classList.toggle('selected', state.selected === node.id);
      node.element.classList.toggle('dim', !!state.selected && !neighbors.has(node.id));
    });
    visibleEdges.forEach(edge => {
      const adjacent = edge.source === state.selected || edge.target === state.selected;
      edge.element.classList.toggle('dim', !!state.selected && !adjacent);
      edge.element.classList.toggle('emphasized', adjacent);
      edge.text.textContent = adjacent ? [...new Set(activeRelations(edge).map(r => types[r.type] + (r.condition ? ' · 有条件' : '')))].join(' / ') : '';
      edge.line.setAttribute('stroke', colors[activeRelations(edge)[0].type]);
    });
  }
  function rows(object) {
    return `<dl>${Object.entries(object).map(([key, value]) => `<div class="contract-row"><dt>${escape(fieldNames[key] || key)}</dt><dd>${escape(displayValue(value))}</dd></div>`).join('')}</dl>`;
  }
  function select(id, rerender = true) {
    state.selected = id;
    $('overview').hidden = !!id; $('detail').hidden = !id; $('neighbors').disabled = !id;
    if (!id) { $('neighbors').checked = false; $('detail').replaceChildren(); }
    else {
      const node = nodeById.get(id), related = data.edges.filter(edge => edge.source === id || edge.target === id);
      const header = node.header.replace(/^include\//, '');
      $('detail').innerHTML = `<div class="detail-kind"><span class="kind-dot ${escape(node.kind)}"></span>${escape(kinds[node.kind])}</div><h2 class="detail-title">${escape(node.label)}</h2><div class="asset-id">${escape(id)}</div><p class="detail-name">${escape(node.name)}<br>问题：${escape(node.problem)}</p><div class="detail-section"><h3>代码入口 · C++20</h3><pre class="include-code">#include &lt;${escape(header)}&gt;</pre><div class="detail-actions"><button id="copy-include">复制 include</button><button id="show-source">阅读源码</button></div></div><div class="detail-section"><h3>关联资产 · ${related.length}</h3><div id="related-list"></div></div><div class="detail-section"><h3>输入与输出</h3>${rows(node.input)}${rows(node.output)}</div><div class="detail-section"><h3>时间与空间</h3>${rows(node.complexity)}</div><div class="detail-section"><h3>状态能力</h3>${Object.entries(node.capabilities).map(([key, value]) => `<div class="capability"><span>${escape(capabilities[key] || key)}</span><span class="${escape(value)}">${escape(statuses[value])}</span></div>`).join('')}</div><div class="detail-section"><details><summary>前置条件与约束</summary>${rows(node.requires)}${rows(node.constraints)}</details></div><div class="detail-section"><details><summary>证据与来源</summary>${rows(node.evidence)}${rows(node.provenance)}<p class="asset-id">${escape(node.documentation)}</p></details></div>`;
      if (!related.length) $('related-list').innerHTML = '<p class="detail-name">暂未登记关联。可在契约 relations 中补充经过验证的关系。</p>';
      related.forEach(edge => {
        const other = edge.source === id ? edge.target : edge.source, button = document.createElement('button');
        button.className = 'relation-item';
        button.innerHTML = `${escape(label(other))}<span>${edge.relations.map(r => `${escape(label(r.source))} ${r.type === 'alternative' ? '↔' : '→'} ${escape(label(r.target))} · ${escape(types[r.type])}${r.condition ? `<br>条件：${escape(r.condition)}` : ''}`).join('<br>')}</span>`;
        button.addEventListener('click', () => {
          // A relation can point outside the active kind/search filter.
          $('search').value = ''; selectedKinds.add(nodeById.get(other).kind);
          document.querySelectorAll('#kind-filters input').forEach(input => { input.checked = selectedKinds.has(input.dataset.filter); });
          select(other);
        });
        $('related-list').append(button);
      });
      $('copy-include').addEventListener('click', async () => {
        const snippet = `#include <${header}>`;
        try { await navigator.clipboard.writeText(snippet); notice('已复制 include'); }
        catch { notice('当前浏览器不允许复制，请从代码框选中复制。'); }
      });
      $('show-source').addEventListener('click', () => {
        $('source-path').textContent = node.header; $('source-code').textContent = node.source;
        $('source-dialog').showModal();
      });
    }
    if (rerender) refresh();
  }
  let noticeTimer;
  function notice(text) { clearTimeout(noticeTimer); $('notice').textContent = text; $('notice').hidden = false; noticeTimer = setTimeout(() => { $('notice').hidden = true; }, 3000); }
  function updateTransform() {
    $('world').setAttribute('transform', `translate(${state.tx} ${state.ty}) scale(${state.scale})`);
    $('zoom-label').textContent = `${Math.round(state.scale * 100)}%`;
    // Keep labels readable independently of graph zoom, like a knowledge graph.
    data.nodes.forEach(node => {
      const text = node.element.querySelector('text');
      text.style.fontSize = `${12 / state.scale}px`;
      text.style.strokeWidth = `${5 / state.scale}px`;
      text.setAttribute('y', 27 / state.scale);
    });
    data.edges.forEach(edge => {
      edge.text.style.fontSize = `${10 / state.scale}px`;
      edge.text.style.strokeWidth = `${6 / state.scale}px`;
    });
    updatePositions();
  }
  function updatePositions() {
    visibleNodes.forEach(node => { node.element.setAttribute('transform', `translate(${node.x} ${node.y})`); });
    visibleEdges.forEach(edge => {
      const a = nodeById.get(edge.source), b = nodeById.get(edge.target);
      [edge.line, edge.hit].forEach(line => { line.setAttribute('x1', a.x); line.setAttribute('y1', a.y); line.setAttribute('x2', b.x); line.setAttribute('y2', b.y); });
      edge.text.setAttribute('x', (a.x + b.x) / 2); edge.text.setAttribute('y', (a.y + b.y) / 2 - 12 / state.scale);
    });
  }
  function layout() {
    frame = null;
    if (state.paused || state.ticks <= 0) return;
    state.ticks--;
    const alpha = Math.max(.04, state.ticks / 220);
    visibleNodes.forEach(node => {
      node.vx -= node.x * .001 * alpha; node.vy -= node.y * .001 * alpha;
    });
    for (let i = 0; i < visibleNodes.length; i++) for (let j = i + 1; j < visibleNodes.length; j++) {
      const a = visibleNodes[i], b = visibleNodes[j];
      const dx = b.x - a.x || .1, dy = b.y - a.y || .1, distance = Math.max(30, Math.hypot(dx, dy));
      const force = Math.min(3.5, 4000 / (distance * distance)) * alpha;
      a.vx -= dx / distance * force; a.vy -= dy / distance * force;
      b.vx += dx / distance * force; b.vy += dy / distance * force;
    }
    visibleEdges.forEach(edge => {
      const a = nodeById.get(edge.source), b = nodeById.get(edge.target);
      const dx = b.x - a.x, dy = b.y - a.y, distance = Math.max(1, Math.hypot(dx, dy));
      const force = (distance - 180) * .016 * alpha;
      a.vx += dx / distance * force; a.vy += dy / distance * force;
      b.vx -= dx / distance * force; b.vy -= dy / distance * force;
    });
    visibleNodes.forEach(node => {
      if (drag?.node === node) { node.vx = node.vy = 0; return; }
      node.vx *= .78; node.vy *= .78; node.x += node.vx; node.y += node.vy;
    });
    updatePositions();
    if (state.ticks > 0) frame = requestAnimationFrame(layout);
    else if (state.initialFit) { state.initialFit = false; fit(); }
  }
  function startLayout() { state.ticks = 220; if (!state.paused && !frame) frame = requestAnimationFrame(layout); }
  function fit() {
    if (!visibleNodes.length) return;
    const xs = visibleNodes.map(n => n.x), ys = visibleNodes.map(n => n.y);
    const minX = Math.min(...xs) - 90, maxX = Math.max(...xs) + 90;
    const minY = Math.min(...ys) - 50, maxY = Math.max(...ys) + 70;
    const top = 140, bottom = 130;
    state.scale = Math.min(1.5, Math.max(.25, Math.min((width - 36) / (maxX - minX), Math.max(140, height - top - bottom) / (maxY - minY))));
    state.tx = width / 2 - (minX + maxX) / 2 * state.scale;
    state.ty = top + (height - top - bottom) / 2 - (minY + maxY) / 2 * state.scale;
    updateTransform();
  }
  function zoom(factor, x = width / 2, y = height / 2) {
    state.initialFit = false;
    const next = Math.max(.2, Math.min(4, state.scale * factor)), ratio = next / state.scale;
    state.tx = x - (x - state.tx) * ratio; state.ty = y - (y - state.ty) * ratio;
    state.scale = next; updateTransform();
  }
  $('viewport').addEventListener('wheel', event => {
    event.preventDefault(); const box = $('viewport').getBoundingClientRect();
    zoom(Math.exp(-event.deltaY * .0015), event.clientX - box.left, event.clientY - box.top);
  }, {passive: false});
  $('viewport').addEventListener('pointerdown', event => {
    state.initialFit = false;
    if (event.button !== 0 || event.target.closest('button,input,label,.graph-toolbar')) return;
    $('viewport').setPointerCapture(event.pointerId);
    drag = {startX: event.clientX, startY: event.clientY, tx: state.tx, ty: state.ty, moved: false, pointer: event.pointerId};
  });
  $('viewport').addEventListener('pointermove', event => {
    if (!drag || drag.pointer !== event.pointerId) return;
    if (Math.hypot(event.clientX - drag.startX, event.clientY - drag.startY) > 4) drag.moved = true;
    if (drag.node) {
      if (!drag.moved) return;
      const box = $('viewport').getBoundingClientRect();
      drag.node.x = (event.clientX - box.left - state.tx) / state.scale;
      drag.node.y = (event.clientY - box.top - state.ty) / state.scale;
      updatePositions(); startLayout();
    } else {
      state.tx = drag.tx + event.clientX - drag.startX; state.ty = drag.ty + event.clientY - drag.startY; updateTransform();
    }
  });
  $('viewport').addEventListener('pointerup', event => {
    if (!drag || drag.pointer !== event.pointerId) return;
    const previous = drag; drag = null;
    if (!previous.moved) select(previous.node?.id || null);
  });
  $('viewport').addEventListener('pointercancel', () => { drag = null; });
  $('viewport').addEventListener('keydown', event => {
    if (event.target !== $('viewport')) return;
    if (event.key === '+' || event.key === '=') zoom(1.2);
    else if (event.key === '-') zoom(1 / 1.2);
    else if (event.key === '0') fit();
    else if (['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(event.key)) {
      state.tx += event.key === 'ArrowLeft' ? 30 : event.key === 'ArrowRight' ? -30 : 0;
      state.ty += event.key === 'ArrowUp' ? 30 : event.key === 'ArrowDown' ? -30 : 0;
      updateTransform();
    } else return;
    event.preventDefault();
  });
  $('zoom-in').addEventListener('click', () => zoom(1.2));
  $('zoom-out').addEventListener('click', () => zoom(1 / 1.2));
  $('fit').addEventListener('click', fit);
  function motionLabel() { $('motion').textContent = state.paused ? '继续布局' : '暂停布局'; $('motion').setAttribute('aria-pressed', state.paused); }
  $('motion').addEventListener('click', () => { state.paused = !state.paused; motionLabel(); startLayout(); });
  $('search').addEventListener('input', refresh);
  $('neighbors').addEventListener('change', () => { refresh(); fit(); });
  $('deselect').addEventListener('click', () => select(null));
  $('clear-filters').addEventListener('click', () => {
    $('search').value = ''; Object.keys(kinds).forEach(key => selectedKinds.add(key)); Object.keys(types).forEach(key => selectedTypes.add(key));
    document.querySelectorAll('.filters input').forEach(input => { input.checked = true; }); select(null); fit();
  });
  $('close-source').addEventListener('click', () => $('source-dialog').close());
  document.addEventListener('keydown', event => { if (event.key === 'Escape' && !$('source-dialog').open) select(null); });
  new ResizeObserver(() => {
    const box = $('viewport').getBoundingClientRect();
    state.tx += (box.width - width) / 2; state.ty += (box.height - height) / 2;
    width = box.width; height = box.height;
    fit();
  }).observe($('viewport'));
  motionLabel(); refresh();
})();
