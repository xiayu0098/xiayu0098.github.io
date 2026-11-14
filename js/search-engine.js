export function normalize(value) {
  return String(value || '').normalize('NFKC').toLocaleLowerCase().replace(/\s+/g, ' ').trim();
}

export function searchPosts(posts, query) {
  const terms = normalize(query).split(' ').filter(Boolean);
  if (!terms.length) return [];
  return posts.map(post => {
    const title = normalize(post.title);
    const text = normalize(post.text);
    const tags = normalize((post.tags || []).join(' '));
    const haystack = title + ' ' + tags + ' ' + text;
    if (!terms.every(term => haystack.includes(term))) return null;
    const score = terms.reduce((sum, term) => sum + (title.includes(term) ? 10 : 0) + (tags.includes(term) ? 3 : 0), 0);
    return {post, score};
  }).filter(Boolean).sort((a, b) => b.score - a.score).map(result => result.post);
}

export function snippet(text, query, limit = 190) {
  const clean = String(text || '').replace(/\s+/g, ' ').trim();
  const term = normalize(query).split(' ').find(Boolean) || '';
  const index = normalize(clean).indexOf(term);
  const start = Math.max(0, index - 55);
  return (start ? '…' : '') + clean.slice(start, start + limit) + (clean.length > start + limit ? '…' : '');
}
