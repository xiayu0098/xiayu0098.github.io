import {searchPosts, snippet} from './search-engine.js';

const dialog = document.querySelector('#site-search');
const input = dialog.querySelector('input');
const status = dialog.querySelector('[role="status"]');
const results = dialog.querySelector('.search-results');
let indexPromise;
let revision = 0;

function decode(text) {
  const field = document.createElement('textarea');
  field.innerHTML = text || '';
  return field.value;
}

function loadIndex() {
  if (!indexPromise) {
    indexPromise = fetch(dialog.dataset.index).then(response => {
      if (!response.ok) throw new Error('Search index unavailable');
      return response.json();
    }).then(data => data.posts.map(post => ({
      title: decode(post.title), text: decode(post.text), path: post.path,
      tags: (post.tags || []).map(tag => decode(typeof tag === 'string' ? tag : tag.name))
    }))).catch(error => { indexPromise = undefined; throw error; });
  }
  return indexPromise;
}

async function update() {
  const current = ++revision;
  const query = input.value.trim();
  results.replaceChildren();
  if (!query) { status.textContent = 'Search titles and full post content.'; return; }
  status.textContent = 'Searching…';
  try {
    const posts = await loadIndex();
    if (current !== revision) return;
    const matches = searchPosts(posts, query);
    status.textContent = matches.length ? `${matches.length} ${matches.length === 1 ? 'note' : 'notes'} found` : 'No notes found. Try a different word or fewer words.';
    for (const post of matches) {
      const url = new URL(post.path, new URL(dialog.dataset.root, location.origin));
      if (url.origin !== location.origin || !['http:', 'https:'].includes(url.protocol)) continue;
      const item = document.createElement('li');
      const link = document.createElement('a');
      link.href = url.href;
      link.textContent = post.title || 'Untitled';
      const preview = document.createElement('p');
      preview.textContent = snippet(post.text, query);
      item.append(link, preview);
      results.append(item);
    }
  } catch {
    if (current === revision) status.textContent = 'Search could not load. Check your connection, then type again to retry.';
  }
}

document.querySelector('[data-open-search]').addEventListener('click', () => {
  dialog.showModal();
  input.focus();
});
dialog.querySelector('[data-close-search]').addEventListener('click', () => dialog.close());
dialog.addEventListener('click', event => { if (event.target === dialog) dialog.close(); });
dialog.querySelector('form').addEventListener('submit', event => { event.preventDefault(); update(); });
input.addEventListener('input', update);
