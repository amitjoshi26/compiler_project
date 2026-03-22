// ============================================================
// app.js — Compiler Frontend Simulator
// Simulates the C++ compiler phases in the browser:
//   Phase 1: Lexical Analysis
//   Phase 2: Syntax Analysis (Recursive Descent)
//   Phase 3: Symbol Table
//   Phase 4: Semantic Analysis
//   Phase 5: Three Address Code Generation
// ============================================================

"use strict";

// ── KEYWORDS (matches lexer.cpp) ─────────────────────────────
const KEYWORDS = new Set(["int", "float", "if", "else", "while", "return", "void"]);

// ── TOKEN TYPES ───────────────────────────────────────────────
const TK = {
  KEYWORD: "KEYWORD", IDENTIFIER: "IDENTIFIER",
  NUMBER: "NUMBER",   OPERATOR: "OPERATOR",
  SYMBOL: "SYMBOL",   EOF: "EOF",   UNKNOWN: "UNKNOWN"
};

// =============================================================
// PHASE 1 — LEXER
// Mirrors the logic in lexer.cpp
// =============================================================
function tokenize(source) {
  const tokens = [];
  let pos = 0, line = 1;

  const peek = () => pos < source.length ? source[pos] : '\0';
  const adv  = () => {
    const c = source[pos++];
    if (c === '\n') line++;
    return c;
  };

  while (pos < source.length) {
    // Skip whitespace
    while (pos < source.length && /\s/.test(peek())) adv();
    if (pos >= source.length) break;

    let c = peek();

    // Single-line comment
    if (c === '/' && source[pos+1] === '/') {
      while (pos < source.length && peek() !== '\n') adv();
      continue;
    }

    // Number
    if (/[0-9]/.test(c)) {
      let num = '';
      while (pos < source.length && /[0-9]/.test(peek())) num += adv();
      tokens.push({ type: TK.NUMBER, value: num, line });
      continue;
    }

    // Identifier or Keyword
    if (/[a-zA-Z_]/.test(c)) {
      let word = '';
      while (pos < source.length && /[a-zA-Z0-9_]/.test(peek())) word += adv();
      tokens.push({ type: KEYWORDS.has(word) ? TK.KEYWORD : TK.IDENTIFIER, value: word, line });
      continue;
    }

    // String literal
    if (c === '"') {
      adv();
      let str = '';
      while (pos < source.length && peek() !== '"') str += adv();
      adv();
      tokens.push({ type: TK.IDENTIFIER, value: `"${str}"`, line });
      continue;
    }

    adv(); // consume the character
    const op = c;
    const nxt = peek();

    // Two-char operators
    if ((c==='=' && nxt==='=') || (c==='!' && nxt==='=') ||
        (c==='<' && nxt==='=') || (c==='>' && nxt==='=')) {
      tokens.push({ type: TK.OPERATOR, value: op + adv(), line });
      continue;
    }

    if ('+-*/=<>'.includes(c)) {
      tokens.push({ type: TK.OPERATOR, value: op, line });
      continue;
    }

    if ('(){}[];,'.includes(c)) {
      tokens.push({ type: TK.SYMBOL, value: op, line });
      continue;
    }

    tokens.push({ type: TK.UNKNOWN, value: op, line });
  }

  tokens.push({ type: TK.EOF, value: 'EOF', line });
  return tokens;
}

// =============================================================
// PHASE 2–5 — PARSER, SYMBOL TABLE, SEMANTIC, TAC
// Mirrors parser.cpp + symbol_table.h + semantic.h + tac.h
// =============================================================
function runAllPhases(source) {
  const tokens = tokenize(source);
  const symTable = {};   // { name: { type, line } }
  const parseLog = [];
  const semLog   = [];
  const tacInstr = [];
  let   pos = 0;
  let   tempCount = 0;
  let   semErrors = 0;

  const peek  = () => tokens[pos];
  const adv   = () => tokens[pos < tokens.length - 1 ? pos++ : pos];
  const isEOF = () => peek().type === TK.EOF;

  function expect(type, val) {
    const t = peek();
    if (t.type !== type || (val && t.value !== val)) {
      throw new Error(`Syntax Error line ${t.line}: Expected '${val || type}' got '${t.value}'`);
    }
    return adv();
  }

  function match(type, val) {
    const t = peek();
    if (t.type === type && (!val || t.value === val)) { adv(); return true; }
    return false;
  }

  function isType(v) { return v === 'int' || v === 'float' || v === 'void'; }

  // ── TAC helpers ──
  function newTemp() { return `t${++tempCount}`; }
  function emitTac(r, a1, op, a2) {
    tacInstr.push({ result: r, arg1: a1, op, arg2: a2 });
    return r;
  }

  // ── Semantic helpers ──
  function semCheck(name, line) {
    if (!symTable[name]) {
      semLog.push({ cls: 'log-error', text: `ERROR line ${line}: '${name}' used before declaration.` });
      semErrors++;
    }
  }

  // ── Expression parser (produces TAC) ──
  function parseExpr() {
    let left = parseTerm();
    while (!isEOF() && (peek().value === '+' || peek().value === '-')) {
      const op = adv().value;
      const right = parseTerm();
      const tmp = newTemp();
      emitTac(tmp, left, op, right);
      left = tmp;
    }
    return left;
  }

  function parseTerm() {
    let left = parseFactor();
    while (!isEOF() && (peek().value === '*' || peek().value === '/')) {
      const op = adv().value;
      const right = parseFactor();
      const tmp = newTemp();
      emitTac(tmp, left, op, right);
      left = tmp;
    }
    return left;
  }

  function parseFactor() {
    const t = peek();
    if (t.type === TK.NUMBER)     { adv(); return t.value; }
    if (t.type === TK.IDENTIFIER) { adv(); semCheck(t.value, t.line); return t.value; }
    if (t.value === '(') {
      adv(); const v = parseExpr(); expect(TK.SYMBOL, ')'); return v;
    }
    throw new Error(`Syntax Error line ${t.line}: Unexpected '${t.value}' in expression`);
  }

  // ── Statement parsers ──
  function parseDeclStatement() {
    const typeToken = adv();
    const nameToken = expect(TK.IDENTIFIER, '');
    parseLog.push({ cls: 'log-decl', text: `Declaration: ${typeToken.value} ${nameToken.value}` });

    if (symTable[nameToken.value]) {
      semLog.push({ cls: 'log-error', text: `ERROR line ${nameToken.line}: '${nameToken.value}' already declared.` });
      semErrors++;
    } else {
      symTable[nameToken.value] = { type: typeToken.value, line: nameToken.line };
    }

    if (peek().value === '=') {
      adv();
      const initVal = parseExpr();
      parseLog.push({ cls: 'log-assign', text: `  Initialized with: ${initVal}` });
      emitTac(nameToken.value, initVal, '=', '');
    }
    expect(TK.SYMBOL, ';');
  }

  function parseAssignStatement() {
    const nameToken = adv();
    if (!symTable[nameToken.value]) {
      semLog.push({ cls: 'log-error', text: `ERROR line ${nameToken.line}: '${nameToken.value}' used before declaration.` });
      semErrors++;
    }
    expect(TK.OPERATOR, '=');
    const val = parseExpr();
    parseLog.push({ cls: 'log-assign', text: `Assignment: ${nameToken.value} = ${val}` });
    emitTac(nameToken.value, val, '=', '');
    expect(TK.SYMBOL, ';');
  }

  function parseStatement() {
    const t = peek();
    if (t.type === TK.KEYWORD && isType(t.value)) parseDeclStatement();
    else if (t.type === TK.IDENTIFIER)             parseAssignStatement();
    else {
      parseLog.push({ cls: 'log-warn', text: `Warning line ${t.line}: Unexpected '${t.value}'. Skipping.` });
      adv();
    }
  }

  // ── Run parser ──
  let parseError = null;
  try {
    while (!isEOF()) parseStatement();
    parseLog.push({ cls: 'log-ok', text: '✓ Parsing completed successfully.' });
  } catch (e) {
    parseError = e.message;
    parseLog.push({ cls: 'log-error', text: parseError });
  }

  // ── Semantic summary ──
  if (semErrors === 0) semLog.push({ cls: 'log-ok', text: '✓ No semantic errors found.' });
  else semLog.push({ cls: 'log-error', text: `${semErrors} semantic error(s) found.` });

  return { tokens, symTable, parseLog, semLog, tacInstr };
}

// =============================================================
// UI — Render results into the DOM
// =============================================================

function renderTokens(tokens) {
  const tbody = document.getElementById('tokenBody');
  tbody.innerHTML = '';
  tokens.forEach(tok => {
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${tok.line}</td>
      <td class="tk-${tok.type}">${tok.type}</td>
      <td>${escHtml(tok.value)}</td>`;
    tbody.appendChild(tr);
  });
}

function renderParseLog(log) {
  const el = document.getElementById('parseLog');
  el.innerHTML = log.map(e => `<span class="${e.cls}">${escHtml(e.text)}</span>`).join('\n');
}

function renderSymTable(symTable) {
  const tbody = document.getElementById('symBody');
  tbody.innerHTML = '';
  const entries = Object.entries(symTable);
  if (!entries.length) {
    tbody.innerHTML = '<tr><td colspan="3" class="placeholder">No symbols found.</td></tr>';
    return;
  }
  entries.forEach(([name, sym]) => {
    const tr = document.createElement('tr');
    tr.innerHTML = `<td class="tk-IDENTIFIER">${escHtml(name)}</td><td class="tk-KEYWORD">${sym.type}</td><td>${sym.line}</td>`;
    tbody.appendChild(tr);
  });
}

function renderSemantic(log) {
  const el = document.getElementById('semanticLog');
  el.innerHTML = log.map(e => `<span class="${e.cls}">${escHtml(e.text)}</span>`).join('\n');
}

function renderTAC(instr) {
  const el = document.getElementById('tacOutput');
  if (!instr.length) { el.innerHTML = '<span class="placeholder-text">(No TAC generated)</span>'; return; }
  el.innerHTML = instr.map(i => {
    if (i.op === '=') {
      return `<span class="tac-instr">  <span class="tac-temp">${escHtml(i.result)}</span> = ${escHtml(i.arg1)}</span>`;
    }
    return `<span class="tac-instr">  <span class="tac-temp">${escHtml(i.result)}</span> = ${escHtml(i.arg1)} <span class="tac-op">${i.op}</span> ${escHtml(i.arg2)}</span>`;
  }).join('\n');
}

function escHtml(s) {
  return String(s)
    .replace(/&/g,'&amp;').replace(/</g,'&lt;')
    .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

// =============================================================
// FILE TREE DATA
// =============================================================
const FILES = [
  { icon: '📄', name: 'main.cpp',         phase: 'Driver',      desc: 'Entry point. Runs all phases in sequence.' },
  { icon: '🔤', name: 'lexer.h',           phase: 'Phase 1',     desc: 'Token struct, Lexer class declaration, TokenType enum.' },
  { icon: '🔤', name: 'lexer.cpp',         phase: 'Phase 1',     desc: 'Tokenizer: reads keywords, identifiers, numbers, operators, symbols.' },
  { icon: '🌳', name: 'parser.h',          phase: 'Phase 2',     desc: 'Parser class declaration — recursive-descent grammar.' },
  { icon: '🌳', name: 'parser.cpp',        phase: 'Phase 2',     desc: 'parseStatement, parseDeclStatement, parseExpr, parseTerm, parseFactor.' },
  { icon: '📋', name: 'symbol_table.h',    phase: 'Phase 3',     desc: 'Symbol struct, insert/lookup/print methods, unordered_map storage.' },
  { icon: '🔍', name: 'semantic.h',        phase: 'Phase 4',     desc: 'checkDeclared, checkTypes, checkAssignTypes, error reporting.' },
  { icon: '⚙️', name: 'tac.h',             phase: 'Phase 5',     desc: 'TACInstruction struct, newTemp, emit, emitCopy, print methods.' },
  { icon: '📝', name: 'sample_input.txt',  phase: 'Input',       desc: 'Sample source program for testing the compiler.' },
];

function renderFileTree() {
  const el = document.getElementById('fileTree');
  el.innerHTML = FILES.map(f => `
    <div class="file-entry">
      <span class="file-icon">${f.icon}</span>
      <div style="flex:1">
        <div class="file-name">${f.name}</div>
        <div class="file-desc">${f.desc}</div>
      </div>
      <span class="file-phase">${f.phase}</span>
    </div>`).join('');
}

// =============================================================
// TABS
// =============================================================
document.getElementById('phaseNav').addEventListener('click', e => {
  const btn = e.target.closest('.phase-btn');
  if (!btn) return;
  const phase = btn.dataset.phase;
  document.querySelectorAll('.phase-btn').forEach(b => b.classList.remove('active'));
  document.querySelectorAll('.phase-panel').forEach(p => p.classList.remove('active'));
  btn.classList.add('active');
  document.getElementById(`panel-${phase}`).classList.add('active');
});

// =============================================================
// RUN BUTTON
// =============================================================
document.getElementById('runBtn').addEventListener('click', () => {
  const source = document.getElementById('sourceInput').value;
  try {
    const { tokens, symTable, parseLog, semLog, tacInstr } = runAllPhases(source);
    renderTokens(tokens);
    renderParseLog(parseLog);
    renderSymTable(symTable);
    renderSemantic(semLog);
    renderTAC(tacInstr);
  } catch (err) {
    document.getElementById('parseLog').innerHTML =
      `<span class="log-error">Fatal Error: ${escHtml(err.message)}</span>`;
  }
});

// =============================================================
// INIT
// =============================================================
renderFileTree();
// Auto-run on load with default input
document.getElementById('runBtn').click();
